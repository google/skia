//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#include <hs_cl_macros.h>

//
//
//

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_transpose(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vout + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vout + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vout + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vout + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vout + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vout + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vout + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vout + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vout + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vout + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vout + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vout + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vout + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vout + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vout + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vout + gmem_idx)[15 * 8];
  HS_TRANSPOSE_SLAB()
}

__kernel __attribute__((reqd_work_group_size(128, 1, 1)))
__attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bs_4(__global HS_KEY_TYPE const* const restrict vin,
               __global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 128];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vin + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vin + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vin + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vin + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vin + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vin + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vin + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vin + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vin + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vin + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vin + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vin + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vin + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vin + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vin + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vin + gmem_idx)[15 * 8];
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r6, r11)
  HS_CMP_XCHG(r7, r10)
  HS_CMP_XCHG(r4, r13)
  HS_CMP_XCHG(r14, r15)
  HS_CMP_XCHG(r8, r12)
  HS_CMP_XCHG(r2, r3)
  HS_CMP_XCHG(r5, r9)
  HS_CMP_XCHG(r2, r5)
  HS_CMP_XCHG(r8, r14)
  HS_CMP_XCHG(r3, r9)
  HS_CMP_XCHG(r12, r15)
  HS_CMP_XCHG(r3, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r14)
  HS_CMP_XCHG(r4, r9)
  HS_CMP_XCHG(r8, r13)
  HS_CMP_XCHG(r7, r9)
  HS_CMP_XCHG(r11, r13)
  HS_CMP_XCHG(r4, r6)
  HS_CMP_XCHG(r8, r10)
  HS_CMP_XCHG(r4, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r8, r9)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r13)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  {
    uint const flip_lane_mask = 1;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 3;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 7;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 2;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  uint const smem_l_idx = get_sub_group_id() * 128 + get_sub_group_local_id();
  uint const smem_r_idx =
    (get_sub_group_id() ^ 1) * 128 + (get_sub_group_local_id() ^ 7);
  (shared.m + get_local_id(0))[16 * 8 * 0] = r1;
  (shared.m + get_local_id(0))[16 * 8 * 1] = r16;
  (shared.m + get_local_id(0))[16 * 8 * 2] = r2;
  (shared.m + get_local_id(0))[16 * 8 * 3] = r15;
  (shared.m + get_local_id(0))[16 * 8 * 4] = r3;
  (shared.m + get_local_id(0))[16 * 8 * 5] = r14;
  (shared.m + get_local_id(0))[16 * 8 * 6] = r4;
  (shared.m + get_local_id(0))[16 * 8 * 7] = r13;
  (shared.m + get_local_id(0))[16 * 8 * 8] = r5;
  (shared.m + get_local_id(0))[16 * 8 * 9] = r12;
  (shared.m + get_local_id(0))[16 * 8 * 10] = r6;
  (shared.m + get_local_id(0))[16 * 8 * 11] = r11;
  (shared.m + get_local_id(0))[16 * 8 * 12] = r7;
  (shared.m + get_local_id(0))[16 * 8 * 13] = r10;
  (shared.m + get_local_id(0))[16 * 8 * 14] = r8;
  (shared.m + get_local_id(0))[16 * 8 * 15] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[8];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_r_idx)[8] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[16];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[24];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[16] = r1_1;
      (shared.m + smem_r_idx)[24] = r1_2;
    }
    {
      HS_KEY_TYPE r2_1 = (shared.m + smem_l_idx)[32];
      HS_KEY_TYPE r2_2 = (shared.m + smem_r_idx)[40];
      HS_CMP_XCHG(r2_1, r2_2)
      (shared.m + smem_l_idx)[32] = r2_1;
      (shared.m + smem_r_idx)[40] = r2_2;
    }
    {
      HS_KEY_TYPE r3_1 = (shared.m + smem_l_idx)[48];
      HS_KEY_TYPE r3_2 = (shared.m + smem_r_idx)[56];
      HS_CMP_XCHG(r3_1, r3_2)
      (shared.m + smem_l_idx)[48] = r3_1;
      (shared.m + smem_r_idx)[56] = r3_2;
    }
    {
      HS_KEY_TYPE r4_1 = (shared.m + smem_l_idx)[64];
      HS_KEY_TYPE r4_2 = (shared.m + smem_r_idx)[72];
      HS_CMP_XCHG(r4_1, r4_2)
      (shared.m + smem_l_idx)[64] = r4_1;
      (shared.m + smem_r_idx)[72] = r4_2;
    }
    {
      HS_KEY_TYPE r5_1 = (shared.m + smem_l_idx)[80];
      HS_KEY_TYPE r5_2 = (shared.m + smem_r_idx)[88];
      HS_CMP_XCHG(r5_1, r5_2)
      (shared.m + smem_l_idx)[80] = r5_1;
      (shared.m + smem_r_idx)[88] = r5_2;
    }
    {
      HS_KEY_TYPE r6_1 = (shared.m + smem_l_idx)[96];
      HS_KEY_TYPE r6_2 = (shared.m + smem_r_idx)[104];
      HS_CMP_XCHG(r6_1, r6_2)
      (shared.m + smem_l_idx)[96] = r6_1;
      (shared.m + smem_r_idx)[104] = r6_2;
    }
    {
      HS_KEY_TYPE r7_1 = (shared.m + smem_l_idx)[112];
      HS_KEY_TYPE r7_2 = (shared.m + smem_r_idx)[120];
      HS_CMP_XCHG(r7_1, r7_2)
      (shared.m + smem_l_idx)[112] = r7_1;
      (shared.m + smem_r_idx)[120] = r7_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = (shared.m + get_local_id(0))[16 * 8 * 0];
  r16 = (shared.m + get_local_id(0))[16 * 8 * 1];
  r2 = (shared.m + get_local_id(0))[16 * 8 * 2];
  r15 = (shared.m + get_local_id(0))[16 * 8 * 3];
  r3 = (shared.m + get_local_id(0))[16 * 8 * 4];
  r14 = (shared.m + get_local_id(0))[16 * 8 * 5];
  r4 = (shared.m + get_local_id(0))[16 * 8 * 6];
  r13 = (shared.m + get_local_id(0))[16 * 8 * 7];
  r5 = (shared.m + get_local_id(0))[16 * 8 * 8];
  r12 = (shared.m + get_local_id(0))[16 * 8 * 9];
  r6 = (shared.m + get_local_id(0))[16 * 8 * 10];
  r11 = (shared.m + get_local_id(0))[16 * 8 * 11];
  r7 = (shared.m + get_local_id(0))[16 * 8 * 12];
  r10 = (shared.m + get_local_id(0))[16 * 8 * 13];
  r8 = (shared.m + get_local_id(0))[16 * 8 * 14];
  r9 = (shared.m + get_local_id(0))[16 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[16 * 8 * 0] = r1;
(shared.m + get_local_id(0))[16 * 8 * 1] = r16;
(shared.m + get_local_id(0))[16 * 8 * 2] = r2;
(shared.m + get_local_id(0))[16 * 8 * 3] = r15;
(shared.m + get_local_id(0))[16 * 8 * 4] = r3;
(shared.m + get_local_id(0))[16 * 8 * 5] = r14;
(shared.m + get_local_id(0))[16 * 8 * 6] = r4;
(shared.m + get_local_id(0))[16 * 8 * 7] = r13;
(shared.m + get_local_id(0))[16 * 8 * 8] = r5;
(shared.m + get_local_id(0))[16 * 8 * 9] = r12;
(shared.m + get_local_id(0))[16 * 8 * 10] = r6;
(shared.m + get_local_id(0))[16 * 8 * 11] = r11;
(shared.m + get_local_id(0))[16 * 8 * 12] = r7;
(shared.m + get_local_id(0))[16 * 8 * 13] = r10;
(shared.m + get_local_id(0))[16 * 8 * 14] = r8;
(shared.m + get_local_id(0))[16 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[24];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_r_idx)[16] = r0_3;
    (shared.m + smem_r_idx)[24] = r0_4;
  }
  {
    HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[32];
    HS_KEY_TYPE r1_2 = (shared.m + smem_l_idx)[40];
    HS_KEY_TYPE r1_3 = (shared.m + smem_r_idx)[48];
    HS_KEY_TYPE r1_4 = (shared.m + smem_r_idx)[56];
    HS_CMP_XCHG(r1_2, r1_3)
    HS_CMP_XCHG(r1_1, r1_4)
    HS_CMP_XCHG(r1_3, r1_4)
    HS_CMP_XCHG(r1_1, r1_2)
    (shared.m + smem_l_idx)[32] = r1_1;
    (shared.m + smem_l_idx)[40] = r1_2;
    (shared.m + smem_r_idx)[48] = r1_3;
    (shared.m + smem_r_idx)[56] = r1_4;
  }
  {
    HS_KEY_TYPE r2_1 = (shared.m + smem_l_idx)[64];
    HS_KEY_TYPE r2_2 = (shared.m + smem_l_idx)[72];
    HS_KEY_TYPE r2_3 = (shared.m + smem_r_idx)[80];
    HS_KEY_TYPE r2_4 = (shared.m + smem_r_idx)[88];
    HS_CMP_XCHG(r2_2, r2_3)
    HS_CMP_XCHG(r2_1, r2_4)
    HS_CMP_XCHG(r2_3, r2_4)
    HS_CMP_XCHG(r2_1, r2_2)
    (shared.m + smem_l_idx)[64] = r2_1;
    (shared.m + smem_l_idx)[72] = r2_2;
    (shared.m + smem_r_idx)[80] = r2_3;
    (shared.m + smem_r_idx)[88] = r2_4;
  }
  {
    HS_KEY_TYPE r3_1 = (shared.m + smem_l_idx)[96];
    HS_KEY_TYPE r3_2 = (shared.m + smem_l_idx)[104];
    HS_KEY_TYPE r3_3 = (shared.m + smem_r_idx)[112];
    HS_KEY_TYPE r3_4 = (shared.m + smem_r_idx)[120];
    HS_CMP_XCHG(r3_2, r3_3)
    HS_CMP_XCHG(r3_1, r3_4)
    HS_CMP_XCHG(r3_3, r3_4)
    HS_CMP_XCHG(r3_1, r3_2)
    (shared.m + smem_l_idx)[96] = r3_1;
    (shared.m + smem_l_idx)[104] = r3_2;
    (shared.m + smem_r_idx)[112] = r3_3;
    (shared.m + smem_r_idx)[120] = r3_4;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[16 * 8 * 0];
r16 = (shared.m + get_local_id(0))[16 * 8 * 1];
r2 = (shared.m + get_local_id(0))[16 * 8 * 2];
r15 = (shared.m + get_local_id(0))[16 * 8 * 3];
r3 = (shared.m + get_local_id(0))[16 * 8 * 4];
r14 = (shared.m + get_local_id(0))[16 * 8 * 5];
r4 = (shared.m + get_local_id(0))[16 * 8 * 6];
r13 = (shared.m + get_local_id(0))[16 * 8 * 7];
r5 = (shared.m + get_local_id(0))[16 * 8 * 8];
r12 = (shared.m + get_local_id(0))[16 * 8 * 9];
r6 = (shared.m + get_local_id(0))[16 * 8 * 10];
r11 = (shared.m + get_local_id(0))[16 * 8 * 11];
r7 = (shared.m + get_local_id(0))[16 * 8 * 12];
r10 = (shared.m + get_local_id(0))[16 * 8 * 13];
r8 = (shared.m + get_local_id(0))[16 * 8 * 14];
r9 = (shared.m + get_local_id(0))[16 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[16 * 8 * 0] = r1;
(shared.m + get_local_id(0))[16 * 8 * 1] = r16;
(shared.m + get_local_id(0))[16 * 8 * 2] = r2;
(shared.m + get_local_id(0))[16 * 8 * 3] = r15;
(shared.m + get_local_id(0))[16 * 8 * 4] = r3;
(shared.m + get_local_id(0))[16 * 8 * 5] = r14;
(shared.m + get_local_id(0))[16 * 8 * 6] = r4;
(shared.m + get_local_id(0))[16 * 8 * 7] = r13;
(shared.m + get_local_id(0))[16 * 8 * 8] = r5;
(shared.m + get_local_id(0))[16 * 8 * 9] = r12;
(shared.m + get_local_id(0))[16 * 8 * 10] = r6;
(shared.m + get_local_id(0))[16 * 8 * 11] = r11;
(shared.m + get_local_id(0))[16 * 8 * 12] = r7;
(shared.m + get_local_id(0))[16 * 8 * 13] = r10;
(shared.m + get_local_id(0))[16 * 8 * 14] = r8;
(shared.m + get_local_id(0))[16 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_l_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_l_idx)[24];
    HS_KEY_TYPE r0_5 = (shared.m + smem_r_idx)[32];
    HS_KEY_TYPE r0_6 = (shared.m + smem_r_idx)[40];
    HS_KEY_TYPE r0_7 = (shared.m + smem_r_idx)[48];
    HS_KEY_TYPE r0_8 = (shared.m + smem_r_idx)[56];
    HS_CMP_XCHG(r0_4, r0_5)
    HS_CMP_XCHG(r0_3, r0_6)
    HS_CMP_XCHG(r0_2, r0_7)
    HS_CMP_XCHG(r0_1, r0_8)
    HS_CMP_XCHG(r0_5, r0_7)
    HS_CMP_XCHG(r0_6, r0_8)
    HS_CMP_XCHG(r0_5, r0_6)
    HS_CMP_XCHG(r0_7, r0_8)
    HS_CMP_XCHG(r0_1, r0_3)
    HS_CMP_XCHG(r0_2, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    HS_CMP_XCHG(r0_3, r0_4)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_l_idx)[16] = r0_3;
    (shared.m + smem_l_idx)[24] = r0_4;
    (shared.m + smem_r_idx)[32] = r0_5;
    (shared.m + smem_r_idx)[40] = r0_6;
    (shared.m + smem_r_idx)[48] = r0_7;
    (shared.m + smem_r_idx)[56] = r0_8;
  }
  {
    HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[64];
    HS_KEY_TYPE r1_2 = (shared.m + smem_l_idx)[72];
    HS_KEY_TYPE r1_3 = (shared.m + smem_l_idx)[80];
    HS_KEY_TYPE r1_4 = (shared.m + smem_l_idx)[88];
    HS_KEY_TYPE r1_5 = (shared.m + smem_r_idx)[96];
    HS_KEY_TYPE r1_6 = (shared.m + smem_r_idx)[104];
    HS_KEY_TYPE r1_7 = (shared.m + smem_r_idx)[112];
    HS_KEY_TYPE r1_8 = (shared.m + smem_r_idx)[120];
    HS_CMP_XCHG(r1_4, r1_5)
    HS_CMP_XCHG(r1_3, r1_6)
    HS_CMP_XCHG(r1_2, r1_7)
    HS_CMP_XCHG(r1_1, r1_8)
    HS_CMP_XCHG(r1_5, r1_7)
    HS_CMP_XCHG(r1_6, r1_8)
    HS_CMP_XCHG(r1_5, r1_6)
    HS_CMP_XCHG(r1_7, r1_8)
    HS_CMP_XCHG(r1_1, r1_3)
    HS_CMP_XCHG(r1_2, r1_4)
    HS_CMP_XCHG(r1_1, r1_2)
    HS_CMP_XCHG(r1_3, r1_4)
    (shared.m + smem_l_idx)[64] = r1_1;
    (shared.m + smem_l_idx)[72] = r1_2;
    (shared.m + smem_l_idx)[80] = r1_3;
    (shared.m + smem_l_idx)[88] = r1_4;
    (shared.m + smem_r_idx)[96] = r1_5;
    (shared.m + smem_r_idx)[104] = r1_6;
    (shared.m + smem_r_idx)[112] = r1_7;
    (shared.m + smem_r_idx)[120] = r1_8;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[16 * 8 * 0];
r16 = (shared.m + get_local_id(0))[16 * 8 * 1];
r2 = (shared.m + get_local_id(0))[16 * 8 * 2];
r15 = (shared.m + get_local_id(0))[16 * 8 * 3];
r3 = (shared.m + get_local_id(0))[16 * 8 * 4];
r14 = (shared.m + get_local_id(0))[16 * 8 * 5];
r4 = (shared.m + get_local_id(0))[16 * 8 * 6];
r13 = (shared.m + get_local_id(0))[16 * 8 * 7];
r5 = (shared.m + get_local_id(0))[16 * 8 * 8];
r12 = (shared.m + get_local_id(0))[16 * 8 * 9];
r6 = (shared.m + get_local_id(0))[16 * 8 * 10];
r11 = (shared.m + get_local_id(0))[16 * 8 * 11];
r7 = (shared.m + get_local_id(0))[16 * 8 * 12];
r10 = (shared.m + get_local_id(0))[16 * 8 * 13];
r8 = (shared.m + get_local_id(0))[16 * 8 * 14];
r9 = (shared.m + get_local_id(0))[16 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[16 * 8 * 0] = r1;
(shared.m + get_local_id(0))[16 * 8 * 1] = r16;
(shared.m + get_local_id(0))[16 * 8 * 2] = r2;
(shared.m + get_local_id(0))[16 * 8 * 3] = r15;
(shared.m + get_local_id(0))[16 * 8 * 4] = r3;
(shared.m + get_local_id(0))[16 * 8 * 5] = r14;
(shared.m + get_local_id(0))[16 * 8 * 6] = r4;
(shared.m + get_local_id(0))[16 * 8 * 7] = r13;
(shared.m + get_local_id(0))[16 * 8 * 8] = r5;
(shared.m + get_local_id(0))[16 * 8 * 9] = r12;
(shared.m + get_local_id(0))[16 * 8 * 10] = r6;
(shared.m + get_local_id(0))[16 * 8 * 11] = r11;
(shared.m + get_local_id(0))[16 * 8 * 12] = r7;
(shared.m + get_local_id(0))[16 * 8 * 13] = r10;
(shared.m + get_local_id(0))[16 * 8 * 14] = r8;
(shared.m + get_local_id(0))[16 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_l_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_l_idx)[24];
    HS_KEY_TYPE r0_5 = (shared.m + smem_l_idx)[32];
    HS_KEY_TYPE r0_6 = (shared.m + smem_l_idx)[40];
    HS_KEY_TYPE r0_7 = (shared.m + smem_l_idx)[48];
    HS_KEY_TYPE r0_8 = (shared.m + smem_l_idx)[56];
    HS_KEY_TYPE r0_9 = (shared.m + smem_r_idx)[64];
    HS_KEY_TYPE r0_10 = (shared.m + smem_r_idx)[72];
    HS_KEY_TYPE r0_11 = (shared.m + smem_r_idx)[80];
    HS_KEY_TYPE r0_12 = (shared.m + smem_r_idx)[88];
    HS_KEY_TYPE r0_13 = (shared.m + smem_r_idx)[96];
    HS_KEY_TYPE r0_14 = (shared.m + smem_r_idx)[104];
    HS_KEY_TYPE r0_15 = (shared.m + smem_r_idx)[112];
    HS_KEY_TYPE r0_16 = (shared.m + smem_r_idx)[120];
    HS_CMP_XCHG(r0_8, r0_9)
    HS_CMP_XCHG(r0_7, r0_10)
    HS_CMP_XCHG(r0_6, r0_11)
    HS_CMP_XCHG(r0_5, r0_12)
    HS_CMP_XCHG(r0_4, r0_13)
    HS_CMP_XCHG(r0_3, r0_14)
    HS_CMP_XCHG(r0_2, r0_15)
    HS_CMP_XCHG(r0_1, r0_16)
    HS_CMP_XCHG(r0_9, r0_13)
    HS_CMP_XCHG(r0_11, r0_15)
    HS_CMP_XCHG(r0_9, r0_11)
    HS_CMP_XCHG(r0_13, r0_15)
    HS_CMP_XCHG(r0_10, r0_14)
    HS_CMP_XCHG(r0_12, r0_16)
    HS_CMP_XCHG(r0_10, r0_12)
    HS_CMP_XCHG(r0_14, r0_16)
    HS_CMP_XCHG(r0_9, r0_10)
    HS_CMP_XCHG(r0_11, r0_12)
    HS_CMP_XCHG(r0_13, r0_14)
    HS_CMP_XCHG(r0_15, r0_16)
    HS_CMP_XCHG(r0_1, r0_5)
    HS_CMP_XCHG(r0_3, r0_7)
    HS_CMP_XCHG(r0_1, r0_3)
    HS_CMP_XCHG(r0_5, r0_7)
    HS_CMP_XCHG(r0_2, r0_6)
    HS_CMP_XCHG(r0_4, r0_8)
    HS_CMP_XCHG(r0_2, r0_4)
    HS_CMP_XCHG(r0_6, r0_8)
    HS_CMP_XCHG(r0_1, r0_2)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_5, r0_6)
    HS_CMP_XCHG(r0_7, r0_8)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_l_idx)[16] = r0_3;
    (shared.m + smem_l_idx)[24] = r0_4;
    (shared.m + smem_l_idx)[32] = r0_5;
    (shared.m + smem_l_idx)[40] = r0_6;
    (shared.m + smem_l_idx)[48] = r0_7;
    (shared.m + smem_l_idx)[56] = r0_8;
    (shared.m + smem_r_idx)[64] = r0_9;
    (shared.m + smem_r_idx)[72] = r0_10;
    (shared.m + smem_r_idx)[80] = r0_11;
    (shared.m + smem_r_idx)[88] = r0_12;
    (shared.m + smem_r_idx)[96] = r0_13;
    (shared.m + smem_r_idx)[104] = r0_14;
    (shared.m + smem_r_idx)[112] = r0_15;
    (shared.m + smem_r_idx)[120] = r0_16;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[16 * 8 * 0];
r16 = (shared.m + get_local_id(0))[16 * 8 * 1];
r2 = (shared.m + get_local_id(0))[16 * 8 * 2];
r15 = (shared.m + get_local_id(0))[16 * 8 * 3];
r3 = (shared.m + get_local_id(0))[16 * 8 * 4];
r14 = (shared.m + get_local_id(0))[16 * 8 * 5];
r4 = (shared.m + get_local_id(0))[16 * 8 * 6];
r13 = (shared.m + get_local_id(0))[16 * 8 * 7];
r5 = (shared.m + get_local_id(0))[16 * 8 * 8];
r12 = (shared.m + get_local_id(0))[16 * 8 * 9];
r6 = (shared.m + get_local_id(0))[16 * 8 * 10];
r11 = (shared.m + get_local_id(0))[16 * 8 * 11];
r7 = (shared.m + get_local_id(0))[16 * 8 * 12];
r10 = (shared.m + get_local_id(0))[16 * 8 * 13];
r8 = (shared.m + get_local_id(0))[16 * 8 * 14];
r9 = (shared.m + get_local_id(0))[16 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((reqd_work_group_size(64, 1, 1)))
__attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bs_3(__global HS_KEY_TYPE const* const restrict vin,
               __global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 64];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vin + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vin + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vin + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vin + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vin + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vin + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vin + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vin + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vin + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vin + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vin + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vin + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vin + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vin + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vin + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vin + gmem_idx)[15 * 8];
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r6, r11)
  HS_CMP_XCHG(r7, r10)
  HS_CMP_XCHG(r4, r13)
  HS_CMP_XCHG(r14, r15)
  HS_CMP_XCHG(r8, r12)
  HS_CMP_XCHG(r2, r3)
  HS_CMP_XCHG(r5, r9)
  HS_CMP_XCHG(r2, r5)
  HS_CMP_XCHG(r8, r14)
  HS_CMP_XCHG(r3, r9)
  HS_CMP_XCHG(r12, r15)
  HS_CMP_XCHG(r3, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r14)
  HS_CMP_XCHG(r4, r9)
  HS_CMP_XCHG(r8, r13)
  HS_CMP_XCHG(r7, r9)
  HS_CMP_XCHG(r11, r13)
  HS_CMP_XCHG(r4, r6)
  HS_CMP_XCHG(r8, r10)
  HS_CMP_XCHG(r4, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r8, r9)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r13)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  {
    uint const flip_lane_mask = 1;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 3;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 7;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 2;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  uint const smem_l_idx = get_sub_group_id() * 64 + get_sub_group_local_id();
  uint const smem_r_idx =
    (get_sub_group_id() ^ 1) * 64 + (get_sub_group_local_id() ^ 7);
  (shared.m + get_local_id(0))[8 * 8 * 0] = r1;
  (shared.m + get_local_id(0))[8 * 8 * 1] = r16;
  (shared.m + get_local_id(0))[8 * 8 * 2] = r2;
  (shared.m + get_local_id(0))[8 * 8 * 3] = r15;
  (shared.m + get_local_id(0))[8 * 8 * 4] = r3;
  (shared.m + get_local_id(0))[8 * 8 * 5] = r14;
  (shared.m + get_local_id(0))[8 * 8 * 6] = r4;
  (shared.m + get_local_id(0))[8 * 8 * 7] = r13;
  (shared.m + get_local_id(0))[8 * 8 * 8] = r5;
  (shared.m + get_local_id(0))[8 * 8 * 9] = r12;
  (shared.m + get_local_id(0))[8 * 8 * 10] = r6;
  (shared.m + get_local_id(0))[8 * 8 * 11] = r11;
  (shared.m + get_local_id(0))[8 * 8 * 12] = r7;
  (shared.m + get_local_id(0))[8 * 8 * 13] = r10;
  (shared.m + get_local_id(0))[8 * 8 * 14] = r8;
  (shared.m + get_local_id(0))[8 * 8 * 15] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[8];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_r_idx)[8] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[16];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[24];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[16] = r1_1;
      (shared.m + smem_r_idx)[24] = r1_2;
    }
    {
      HS_KEY_TYPE r2_1 = (shared.m + smem_l_idx)[32];
      HS_KEY_TYPE r2_2 = (shared.m + smem_r_idx)[40];
      HS_CMP_XCHG(r2_1, r2_2)
      (shared.m + smem_l_idx)[32] = r2_1;
      (shared.m + smem_r_idx)[40] = r2_2;
    }
    {
      HS_KEY_TYPE r3_1 = (shared.m + smem_l_idx)[48];
      HS_KEY_TYPE r3_2 = (shared.m + smem_r_idx)[56];
      HS_CMP_XCHG(r3_1, r3_2)
      (shared.m + smem_l_idx)[48] = r3_1;
      (shared.m + smem_r_idx)[56] = r3_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[512];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[520];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[512] = r0_1;
      (shared.m + smem_r_idx)[520] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[528];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[536];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[528] = r1_1;
      (shared.m + smem_r_idx)[536] = r1_2;
    }
    {
      HS_KEY_TYPE r2_1 = (shared.m + smem_l_idx)[544];
      HS_KEY_TYPE r2_2 = (shared.m + smem_r_idx)[552];
      HS_CMP_XCHG(r2_1, r2_2)
      (shared.m + smem_l_idx)[544] = r2_1;
      (shared.m + smem_r_idx)[552] = r2_2;
    }
    {
      HS_KEY_TYPE r3_1 = (shared.m + smem_l_idx)[560];
      HS_KEY_TYPE r3_2 = (shared.m + smem_r_idx)[568];
      HS_CMP_XCHG(r3_1, r3_2)
      (shared.m + smem_l_idx)[560] = r3_1;
      (shared.m + smem_r_idx)[568] = r3_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = (shared.m + get_local_id(0))[8 * 8 * 0];
  r16 = (shared.m + get_local_id(0))[8 * 8 * 1];
  r2 = (shared.m + get_local_id(0))[8 * 8 * 2];
  r15 = (shared.m + get_local_id(0))[8 * 8 * 3];
  r3 = (shared.m + get_local_id(0))[8 * 8 * 4];
  r14 = (shared.m + get_local_id(0))[8 * 8 * 5];
  r4 = (shared.m + get_local_id(0))[8 * 8 * 6];
  r13 = (shared.m + get_local_id(0))[8 * 8 * 7];
  r5 = (shared.m + get_local_id(0))[8 * 8 * 8];
  r12 = (shared.m + get_local_id(0))[8 * 8 * 9];
  r6 = (shared.m + get_local_id(0))[8 * 8 * 10];
  r11 = (shared.m + get_local_id(0))[8 * 8 * 11];
  r7 = (shared.m + get_local_id(0))[8 * 8 * 12];
  r10 = (shared.m + get_local_id(0))[8 * 8 * 13];
  r8 = (shared.m + get_local_id(0))[8 * 8 * 14];
  r9 = (shared.m + get_local_id(0))[8 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[8 * 8 * 0] = r1;
(shared.m + get_local_id(0))[8 * 8 * 1] = r16;
(shared.m + get_local_id(0))[8 * 8 * 2] = r2;
(shared.m + get_local_id(0))[8 * 8 * 3] = r15;
(shared.m + get_local_id(0))[8 * 8 * 4] = r3;
(shared.m + get_local_id(0))[8 * 8 * 5] = r14;
(shared.m + get_local_id(0))[8 * 8 * 6] = r4;
(shared.m + get_local_id(0))[8 * 8 * 7] = r13;
(shared.m + get_local_id(0))[8 * 8 * 8] = r5;
(shared.m + get_local_id(0))[8 * 8 * 9] = r12;
(shared.m + get_local_id(0))[8 * 8 * 10] = r6;
(shared.m + get_local_id(0))[8 * 8 * 11] = r11;
(shared.m + get_local_id(0))[8 * 8 * 12] = r7;
(shared.m + get_local_id(0))[8 * 8 * 13] = r10;
(shared.m + get_local_id(0))[8 * 8 * 14] = r8;
(shared.m + get_local_id(0))[8 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[24];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_r_idx)[16] = r0_3;
    (shared.m + smem_r_idx)[24] = r0_4;
  }
  {
    HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[32];
    HS_KEY_TYPE r1_2 = (shared.m + smem_l_idx)[40];
    HS_KEY_TYPE r1_3 = (shared.m + smem_r_idx)[48];
    HS_KEY_TYPE r1_4 = (shared.m + smem_r_idx)[56];
    HS_CMP_XCHG(r1_2, r1_3)
    HS_CMP_XCHG(r1_1, r1_4)
    HS_CMP_XCHG(r1_3, r1_4)
    HS_CMP_XCHG(r1_1, r1_2)
    (shared.m + smem_l_idx)[32] = r1_1;
    (shared.m + smem_l_idx)[40] = r1_2;
    (shared.m + smem_r_idx)[48] = r1_3;
    (shared.m + smem_r_idx)[56] = r1_4;
  }
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[512];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[520];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[528];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[536];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[512] = r0_1;
    (shared.m + smem_l_idx)[520] = r0_2;
    (shared.m + smem_r_idx)[528] = r0_3;
    (shared.m + smem_r_idx)[536] = r0_4;
  }
  {
    HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[544];
    HS_KEY_TYPE r1_2 = (shared.m + smem_l_idx)[552];
    HS_KEY_TYPE r1_3 = (shared.m + smem_r_idx)[560];
    HS_KEY_TYPE r1_4 = (shared.m + smem_r_idx)[568];
    HS_CMP_XCHG(r1_2, r1_3)
    HS_CMP_XCHG(r1_1, r1_4)
    HS_CMP_XCHG(r1_3, r1_4)
    HS_CMP_XCHG(r1_1, r1_2)
    (shared.m + smem_l_idx)[544] = r1_1;
    (shared.m + smem_l_idx)[552] = r1_2;
    (shared.m + smem_r_idx)[560] = r1_3;
    (shared.m + smem_r_idx)[568] = r1_4;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[8 * 8 * 0];
r16 = (shared.m + get_local_id(0))[8 * 8 * 1];
r2 = (shared.m + get_local_id(0))[8 * 8 * 2];
r15 = (shared.m + get_local_id(0))[8 * 8 * 3];
r3 = (shared.m + get_local_id(0))[8 * 8 * 4];
r14 = (shared.m + get_local_id(0))[8 * 8 * 5];
r4 = (shared.m + get_local_id(0))[8 * 8 * 6];
r13 = (shared.m + get_local_id(0))[8 * 8 * 7];
r5 = (shared.m + get_local_id(0))[8 * 8 * 8];
r12 = (shared.m + get_local_id(0))[8 * 8 * 9];
r6 = (shared.m + get_local_id(0))[8 * 8 * 10];
r11 = (shared.m + get_local_id(0))[8 * 8 * 11];
r7 = (shared.m + get_local_id(0))[8 * 8 * 12];
r10 = (shared.m + get_local_id(0))[8 * 8 * 13];
r8 = (shared.m + get_local_id(0))[8 * 8 * 14];
r9 = (shared.m + get_local_id(0))[8 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[8 * 8 * 0] = r1;
(shared.m + get_local_id(0))[8 * 8 * 1] = r16;
(shared.m + get_local_id(0))[8 * 8 * 2] = r2;
(shared.m + get_local_id(0))[8 * 8 * 3] = r15;
(shared.m + get_local_id(0))[8 * 8 * 4] = r3;
(shared.m + get_local_id(0))[8 * 8 * 5] = r14;
(shared.m + get_local_id(0))[8 * 8 * 6] = r4;
(shared.m + get_local_id(0))[8 * 8 * 7] = r13;
(shared.m + get_local_id(0))[8 * 8 * 8] = r5;
(shared.m + get_local_id(0))[8 * 8 * 9] = r12;
(shared.m + get_local_id(0))[8 * 8 * 10] = r6;
(shared.m + get_local_id(0))[8 * 8 * 11] = r11;
(shared.m + get_local_id(0))[8 * 8 * 12] = r7;
(shared.m + get_local_id(0))[8 * 8 * 13] = r10;
(shared.m + get_local_id(0))[8 * 8 * 14] = r8;
(shared.m + get_local_id(0))[8 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_l_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_l_idx)[24];
    HS_KEY_TYPE r0_5 = (shared.m + smem_r_idx)[32];
    HS_KEY_TYPE r0_6 = (shared.m + smem_r_idx)[40];
    HS_KEY_TYPE r0_7 = (shared.m + smem_r_idx)[48];
    HS_KEY_TYPE r0_8 = (shared.m + smem_r_idx)[56];
    HS_CMP_XCHG(r0_4, r0_5)
    HS_CMP_XCHG(r0_3, r0_6)
    HS_CMP_XCHG(r0_2, r0_7)
    HS_CMP_XCHG(r0_1, r0_8)
    HS_CMP_XCHG(r0_5, r0_7)
    HS_CMP_XCHG(r0_6, r0_8)
    HS_CMP_XCHG(r0_5, r0_6)
    HS_CMP_XCHG(r0_7, r0_8)
    HS_CMP_XCHG(r0_1, r0_3)
    HS_CMP_XCHG(r0_2, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    HS_CMP_XCHG(r0_3, r0_4)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_l_idx)[16] = r0_3;
    (shared.m + smem_l_idx)[24] = r0_4;
    (shared.m + smem_r_idx)[32] = r0_5;
    (shared.m + smem_r_idx)[40] = r0_6;
    (shared.m + smem_r_idx)[48] = r0_7;
    (shared.m + smem_r_idx)[56] = r0_8;
  }
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[512];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[520];
    HS_KEY_TYPE r0_3 = (shared.m + smem_l_idx)[528];
    HS_KEY_TYPE r0_4 = (shared.m + smem_l_idx)[536];
    HS_KEY_TYPE r0_5 = (shared.m + smem_r_idx)[544];
    HS_KEY_TYPE r0_6 = (shared.m + smem_r_idx)[552];
    HS_KEY_TYPE r0_7 = (shared.m + smem_r_idx)[560];
    HS_KEY_TYPE r0_8 = (shared.m + smem_r_idx)[568];
    HS_CMP_XCHG(r0_4, r0_5)
    HS_CMP_XCHG(r0_3, r0_6)
    HS_CMP_XCHG(r0_2, r0_7)
    HS_CMP_XCHG(r0_1, r0_8)
    HS_CMP_XCHG(r0_5, r0_7)
    HS_CMP_XCHG(r0_6, r0_8)
    HS_CMP_XCHG(r0_5, r0_6)
    HS_CMP_XCHG(r0_7, r0_8)
    HS_CMP_XCHG(r0_1, r0_3)
    HS_CMP_XCHG(r0_2, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    HS_CMP_XCHG(r0_3, r0_4)
    (shared.m + smem_l_idx)[512] = r0_1;
    (shared.m + smem_l_idx)[520] = r0_2;
    (shared.m + smem_l_idx)[528] = r0_3;
    (shared.m + smem_l_idx)[536] = r0_4;
    (shared.m + smem_r_idx)[544] = r0_5;
    (shared.m + smem_r_idx)[552] = r0_6;
    (shared.m + smem_r_idx)[560] = r0_7;
    (shared.m + smem_r_idx)[568] = r0_8;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[8 * 8 * 0];
r16 = (shared.m + get_local_id(0))[8 * 8 * 1];
r2 = (shared.m + get_local_id(0))[8 * 8 * 2];
r15 = (shared.m + get_local_id(0))[8 * 8 * 3];
r3 = (shared.m + get_local_id(0))[8 * 8 * 4];
r14 = (shared.m + get_local_id(0))[8 * 8 * 5];
r4 = (shared.m + get_local_id(0))[8 * 8 * 6];
r13 = (shared.m + get_local_id(0))[8 * 8 * 7];
r5 = (shared.m + get_local_id(0))[8 * 8 * 8];
r12 = (shared.m + get_local_id(0))[8 * 8 * 9];
r6 = (shared.m + get_local_id(0))[8 * 8 * 10];
r11 = (shared.m + get_local_id(0))[8 * 8 * 11];
r7 = (shared.m + get_local_id(0))[8 * 8 * 12];
r10 = (shared.m + get_local_id(0))[8 * 8 * 13];
r8 = (shared.m + get_local_id(0))[8 * 8 * 14];
r9 = (shared.m + get_local_id(0))[8 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((reqd_work_group_size(32, 1, 1)))
__attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bs_2(__global HS_KEY_TYPE const* const restrict vin,
               __global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 32];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vin + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vin + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vin + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vin + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vin + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vin + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vin + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vin + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vin + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vin + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vin + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vin + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vin + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vin + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vin + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vin + gmem_idx)[15 * 8];
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r6, r11)
  HS_CMP_XCHG(r7, r10)
  HS_CMP_XCHG(r4, r13)
  HS_CMP_XCHG(r14, r15)
  HS_CMP_XCHG(r8, r12)
  HS_CMP_XCHG(r2, r3)
  HS_CMP_XCHG(r5, r9)
  HS_CMP_XCHG(r2, r5)
  HS_CMP_XCHG(r8, r14)
  HS_CMP_XCHG(r3, r9)
  HS_CMP_XCHG(r12, r15)
  HS_CMP_XCHG(r3, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r14)
  HS_CMP_XCHG(r4, r9)
  HS_CMP_XCHG(r8, r13)
  HS_CMP_XCHG(r7, r9)
  HS_CMP_XCHG(r11, r13)
  HS_CMP_XCHG(r4, r6)
  HS_CMP_XCHG(r8, r10)
  HS_CMP_XCHG(r4, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r8, r9)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r13)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  {
    uint const flip_lane_mask = 1;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 3;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 7;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 2;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  uint const smem_l_idx = get_sub_group_id() * 32 + get_sub_group_local_id();
  uint const smem_r_idx =
    (get_sub_group_id() ^ 1) * 32 + (get_sub_group_local_id() ^ 7);
  (shared.m + get_local_id(0))[4 * 8 * 0] = r1;
  (shared.m + get_local_id(0))[4 * 8 * 1] = r16;
  (shared.m + get_local_id(0))[4 * 8 * 2] = r2;
  (shared.m + get_local_id(0))[4 * 8 * 3] = r15;
  (shared.m + get_local_id(0))[4 * 8 * 4] = r3;
  (shared.m + get_local_id(0))[4 * 8 * 5] = r14;
  (shared.m + get_local_id(0))[4 * 8 * 6] = r4;
  (shared.m + get_local_id(0))[4 * 8 * 7] = r13;
  (shared.m + get_local_id(0))[4 * 8 * 8] = r5;
  (shared.m + get_local_id(0))[4 * 8 * 9] = r12;
  (shared.m + get_local_id(0))[4 * 8 * 10] = r6;
  (shared.m + get_local_id(0))[4 * 8 * 11] = r11;
  (shared.m + get_local_id(0))[4 * 8 * 12] = r7;
  (shared.m + get_local_id(0))[4 * 8 * 13] = r10;
  (shared.m + get_local_id(0))[4 * 8 * 14] = r8;
  (shared.m + get_local_id(0))[4 * 8 * 15] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[8];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_r_idx)[8] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[16];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[24];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[16] = r1_1;
      (shared.m + smem_r_idx)[24] = r1_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[128];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[136];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[128] = r0_1;
      (shared.m + smem_r_idx)[136] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[144];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[152];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[144] = r1_1;
      (shared.m + smem_r_idx)[152] = r1_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[256];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[264];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[256] = r0_1;
      (shared.m + smem_r_idx)[264] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[272];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[280];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[272] = r1_1;
      (shared.m + smem_r_idx)[280] = r1_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[384];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[392];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[384] = r0_1;
      (shared.m + smem_r_idx)[392] = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = (shared.m + smem_l_idx)[400];
      HS_KEY_TYPE r1_2 = (shared.m + smem_r_idx)[408];
      HS_CMP_XCHG(r1_1, r1_2)
      (shared.m + smem_l_idx)[400] = r1_1;
      (shared.m + smem_r_idx)[408] = r1_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = (shared.m + get_local_id(0))[4 * 8 * 0];
  r16 = (shared.m + get_local_id(0))[4 * 8 * 1];
  r2 = (shared.m + get_local_id(0))[4 * 8 * 2];
  r15 = (shared.m + get_local_id(0))[4 * 8 * 3];
  r3 = (shared.m + get_local_id(0))[4 * 8 * 4];
  r14 = (shared.m + get_local_id(0))[4 * 8 * 5];
  r4 = (shared.m + get_local_id(0))[4 * 8 * 6];
  r13 = (shared.m + get_local_id(0))[4 * 8 * 7];
  r5 = (shared.m + get_local_id(0))[4 * 8 * 8];
  r12 = (shared.m + get_local_id(0))[4 * 8 * 9];
  r6 = (shared.m + get_local_id(0))[4 * 8 * 10];
  r11 = (shared.m + get_local_id(0))[4 * 8 * 11];
  r7 = (shared.m + get_local_id(0))[4 * 8 * 12];
  r10 = (shared.m + get_local_id(0))[4 * 8 * 13];
  r8 = (shared.m + get_local_id(0))[4 * 8 * 14];
  r9 = (shared.m + get_local_id(0))[4 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(shared.m + get_local_id(0))[4 * 8 * 0] = r1;
(shared.m + get_local_id(0))[4 * 8 * 1] = r16;
(shared.m + get_local_id(0))[4 * 8 * 2] = r2;
(shared.m + get_local_id(0))[4 * 8 * 3] = r15;
(shared.m + get_local_id(0))[4 * 8 * 4] = r3;
(shared.m + get_local_id(0))[4 * 8 * 5] = r14;
(shared.m + get_local_id(0))[4 * 8 * 6] = r4;
(shared.m + get_local_id(0))[4 * 8 * 7] = r13;
(shared.m + get_local_id(0))[4 * 8 * 8] = r5;
(shared.m + get_local_id(0))[4 * 8 * 9] = r12;
(shared.m + get_local_id(0))[4 * 8 * 10] = r6;
(shared.m + get_local_id(0))[4 * 8 * 11] = r11;
(shared.m + get_local_id(0))[4 * 8 * 12] = r7;
(shared.m + get_local_id(0))[4 * 8 * 13] = r10;
(shared.m + get_local_id(0))[4 * 8 * 14] = r8;
(shared.m + get_local_id(0))[4 * 8 * 15] = r9;
barrier(CLK_LOCAL_MEM_FENCE);
{
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[8];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[16];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[24];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[0] = r0_1;
    (shared.m + smem_l_idx)[8] = r0_2;
    (shared.m + smem_r_idx)[16] = r0_3;
    (shared.m + smem_r_idx)[24] = r0_4;
  }
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[128];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[136];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[144];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[152];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[128] = r0_1;
    (shared.m + smem_l_idx)[136] = r0_2;
    (shared.m + smem_r_idx)[144] = r0_3;
    (shared.m + smem_r_idx)[152] = r0_4;
  }
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[256];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[264];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[272];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[280];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[256] = r0_1;
    (shared.m + smem_l_idx)[264] = r0_2;
    (shared.m + smem_r_idx)[272] = r0_3;
    (shared.m + smem_r_idx)[280] = r0_4;
  }
  {
    HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[384];
    HS_KEY_TYPE r0_2 = (shared.m + smem_l_idx)[392];
    HS_KEY_TYPE r0_3 = (shared.m + smem_r_idx)[400];
    HS_KEY_TYPE r0_4 = (shared.m + smem_r_idx)[408];
    HS_CMP_XCHG(r0_2, r0_3)
    HS_CMP_XCHG(r0_1, r0_4)
    HS_CMP_XCHG(r0_3, r0_4)
    HS_CMP_XCHG(r0_1, r0_2)
    (shared.m + smem_l_idx)[384] = r0_1;
    (shared.m + smem_l_idx)[392] = r0_2;
    (shared.m + smem_r_idx)[400] = r0_3;
    (shared.m + smem_r_idx)[408] = r0_4;
  }
}
barrier(CLK_LOCAL_MEM_FENCE);
r1 = (shared.m + get_local_id(0))[4 * 8 * 0];
r16 = (shared.m + get_local_id(0))[4 * 8 * 1];
r2 = (shared.m + get_local_id(0))[4 * 8 * 2];
r15 = (shared.m + get_local_id(0))[4 * 8 * 3];
r3 = (shared.m + get_local_id(0))[4 * 8 * 4];
r14 = (shared.m + get_local_id(0))[4 * 8 * 5];
r4 = (shared.m + get_local_id(0))[4 * 8 * 6];
r13 = (shared.m + get_local_id(0))[4 * 8 * 7];
r5 = (shared.m + get_local_id(0))[4 * 8 * 8];
r12 = (shared.m + get_local_id(0))[4 * 8 * 9];
r6 = (shared.m + get_local_id(0))[4 * 8 * 10];
r11 = (shared.m + get_local_id(0))[4 * 8 * 11];
r7 = (shared.m + get_local_id(0))[4 * 8 * 12];
r10 = (shared.m + get_local_id(0))[4 * 8 * 13];
r8 = (shared.m + get_local_id(0))[4 * 8 * 14];
r9 = (shared.m + get_local_id(0))[4 * 8 * 15];
{ { uint const half_lane_mask = 4;
uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
int const t_lt = get_sub_group_local_id() < half_lane_idx;
HS_CMP_HALF(0, r1)
HS_CMP_HALF(1, r2)
HS_CMP_HALF(2, r3)
HS_CMP_HALF(3, r4)
HS_CMP_HALF(4, r5)
HS_CMP_HALF(5, r6)
HS_CMP_HALF(6, r7)
HS_CMP_HALF(7, r8)
HS_CMP_HALF(8, r9)
HS_CMP_HALF(9, r10)
HS_CMP_HALF(10, r11)
HS_CMP_HALF(11, r12)
HS_CMP_HALF(12, r13)
HS_CMP_HALF(13, r14)
HS_CMP_HALF(14, r15)
HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((reqd_work_group_size(16, 1, 1)))
__attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bs_1(__global HS_KEY_TYPE const* const restrict vin,
               __global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 16];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vin + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vin + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vin + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vin + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vin + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vin + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vin + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vin + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vin + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vin + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vin + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vin + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vin + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vin + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vin + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vin + gmem_idx)[15 * 8];
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r6, r11)
  HS_CMP_XCHG(r7, r10)
  HS_CMP_XCHG(r4, r13)
  HS_CMP_XCHG(r14, r15)
  HS_CMP_XCHG(r8, r12)
  HS_CMP_XCHG(r2, r3)
  HS_CMP_XCHG(r5, r9)
  HS_CMP_XCHG(r2, r5)
  HS_CMP_XCHG(r8, r14)
  HS_CMP_XCHG(r3, r9)
  HS_CMP_XCHG(r12, r15)
  HS_CMP_XCHG(r3, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r14)
  HS_CMP_XCHG(r4, r9)
  HS_CMP_XCHG(r8, r13)
  HS_CMP_XCHG(r7, r9)
  HS_CMP_XCHG(r11, r13)
  HS_CMP_XCHG(r4, r6)
  HS_CMP_XCHG(r8, r10)
  HS_CMP_XCHG(r4, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r8, r9)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r13)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  {
    uint const flip_lane_mask = 1;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 3;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 7;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 2;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  uint const smem_l_idx = get_sub_group_id() * 16 + get_sub_group_local_id();
  uint const smem_r_idx =
    (get_sub_group_id() ^ 1) * 16 + (get_sub_group_local_id() ^ 7);
  (shared.m + get_local_id(0))[2 * 8 * 0] = r1;
  (shared.m + get_local_id(0))[2 * 8 * 1] = r16;
  (shared.m + get_local_id(0))[2 * 8 * 2] = r2;
  (shared.m + get_local_id(0))[2 * 8 * 3] = r15;
  (shared.m + get_local_id(0))[2 * 8 * 4] = r3;
  (shared.m + get_local_id(0))[2 * 8 * 5] = r14;
  (shared.m + get_local_id(0))[2 * 8 * 6] = r4;
  (shared.m + get_local_id(0))[2 * 8 * 7] = r13;
  (shared.m + get_local_id(0))[2 * 8 * 8] = r5;
  (shared.m + get_local_id(0))[2 * 8 * 9] = r12;
  (shared.m + get_local_id(0))[2 * 8 * 10] = r6;
  (shared.m + get_local_id(0))[2 * 8 * 11] = r11;
  (shared.m + get_local_id(0))[2 * 8 * 12] = r7;
  (shared.m + get_local_id(0))[2 * 8 * 13] = r10;
  (shared.m + get_local_id(0))[2 * 8 * 14] = r8;
  (shared.m + get_local_id(0))[2 * 8 * 15] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[8];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_r_idx)[8] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[32];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[40];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[32] = r0_1;
      (shared.m + smem_r_idx)[40] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[64];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[72];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[64] = r0_1;
      (shared.m + smem_r_idx)[72] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[96];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[104];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[96] = r0_1;
      (shared.m + smem_r_idx)[104] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[128];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[136];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[128] = r0_1;
      (shared.m + smem_r_idx)[136] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[160];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[168];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[160] = r0_1;
      (shared.m + smem_r_idx)[168] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[192];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[200];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[192] = r0_1;
      (shared.m + smem_r_idx)[200] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (shared.m + smem_l_idx)[224];
      HS_KEY_TYPE r0_2 = (shared.m + smem_r_idx)[232];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[224] = r0_1;
      (shared.m + smem_r_idx)[232] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = (shared.m + get_local_id(0))[2 * 8 * 0];
  r16 = (shared.m + get_local_id(0))[2 * 8 * 1];
  r2 = (shared.m + get_local_id(0))[2 * 8 * 2];
  r15 = (shared.m + get_local_id(0))[2 * 8 * 3];
  r3 = (shared.m + get_local_id(0))[2 * 8 * 4];
  r14 = (shared.m + get_local_id(0))[2 * 8 * 5];
  r4 = (shared.m + get_local_id(0))[2 * 8 * 6];
  r13 = (shared.m + get_local_id(0))[2 * 8 * 7];
  r5 = (shared.m + get_local_id(0))[2 * 8 * 8];
  r12 = (shared.m + get_local_id(0))[2 * 8 * 9];
  r6 = (shared.m + get_local_id(0))[2 * 8 * 10];
  r11 = (shared.m + get_local_id(0))[2 * 8 * 11];
  r7 = (shared.m + get_local_id(0))[2 * 8 * 12];
  r10 = (shared.m + get_local_id(0))[2 * 8 * 13];
  r8 = (shared.m + get_local_id(0))[2 * 8 * 14];
  r9 = (shared.m + get_local_id(0))[2 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((reqd_work_group_size(8, 1, 1)))
__attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bs_0(__global HS_KEY_TYPE const* const restrict vin,
               __global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vin + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vin + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vin + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vin + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vin + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vin + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vin + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vin + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vin + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vin + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vin + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vin + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vin + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vin + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vin + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vin + gmem_idx)[15 * 8];
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r6, r11)
  HS_CMP_XCHG(r7, r10)
  HS_CMP_XCHG(r4, r13)
  HS_CMP_XCHG(r14, r15)
  HS_CMP_XCHG(r8, r12)
  HS_CMP_XCHG(r2, r3)
  HS_CMP_XCHG(r5, r9)
  HS_CMP_XCHG(r2, r5)
  HS_CMP_XCHG(r8, r14)
  HS_CMP_XCHG(r3, r9)
  HS_CMP_XCHG(r12, r15)
  HS_CMP_XCHG(r3, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r14)
  HS_CMP_XCHG(r4, r9)
  HS_CMP_XCHG(r8, r13)
  HS_CMP_XCHG(r7, r9)
  HS_CMP_XCHG(r11, r13)
  HS_CMP_XCHG(r4, r6)
  HS_CMP_XCHG(r8, r10)
  HS_CMP_XCHG(r4, r5)
  HS_CMP_XCHG(r6, r7)
  HS_CMP_XCHG(r8, r9)
  HS_CMP_XCHG(r10, r11)
  HS_CMP_XCHG(r12, r13)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  {
    uint const flip_lane_mask = 1;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 3;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  {
    uint const flip_lane_mask = 7;
    uint const flip_lane_idx = get_sub_group_local_id() ^ flip_lane_mask;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    HS_CMP_FLIP(0, r1, r16)
    HS_CMP_FLIP(1, r2, r15)
    HS_CMP_FLIP(2, r3, r14)
    HS_CMP_FLIP(3, r4, r13)
    HS_CMP_FLIP(4, r5, r12)
    HS_CMP_FLIP(5, r6, r11)
    HS_CMP_FLIP(6, r7, r10)
    HS_CMP_FLIP(7, r8, r9)
  }
  {
    uint const half_lane_mask = 2;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  {
    uint const half_lane_mask = 1;
    uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    HS_CMP_HALF(0, r1)
    HS_CMP_HALF(1, r2)
    HS_CMP_HALF(2, r3)
    HS_CMP_HALF(3, r4)
    HS_CMP_HALF(4, r5)
    HS_CMP_HALF(5, r6)
    HS_CMP_HALF(6, r7)
    HS_CMP_HALF(7, r8)
    HS_CMP_HALF(8, r9)
    HS_CMP_HALF(9, r10)
    HS_CMP_HALF(10, r11)
    HS_CMP_HALF(11, r12)
    HS_CMP_HALF(12, r13)
    HS_CMP_HALF(13, r14)
    HS_CMP_HALF(14, r15)
    HS_CMP_HALF(15, r16)
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  (vout + gmem_idx)[0 * 8] = r1;
  (vout + gmem_idx)[1 * 8] = r2;
  (vout + gmem_idx)[2 * 8] = r3;
  (vout + gmem_idx)[3 * 8] = r4;
  (vout + gmem_idx)[4 * 8] = r5;
  (vout + gmem_idx)[5 * 8] = r6;
  (vout + gmem_idx)[6 * 8] = r7;
  (vout + gmem_idx)[7 * 8] = r8;
  (vout + gmem_idx)[8 * 8] = r9;
  (vout + gmem_idx)[9 * 8] = r10;
  (vout + gmem_idx)[10 * 8] = r11;
  (vout + gmem_idx)[11 * 8] = r12;
  (vout + gmem_idx)[12 * 8] = r13;
  (vout + gmem_idx)[13 * 8] = r14;
  (vout + gmem_idx)[14 * 8] = r15;
  (vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bc_4(__global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 128];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  uint const gmem_l_idx = (global_id / 128) * 2048 + (global_id & 127);
  uint const smem_l_idx = get_sub_group_id() * 128 + get_sub_group_local_id();
  {
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[128];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[256];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[384];
      HS_KEY_TYPE r0_5 = (vout + gmem_l_idx)[512];
      HS_KEY_TYPE r0_6 = (vout + gmem_l_idx)[640];
      HS_KEY_TYPE r0_7 = (vout + gmem_l_idx)[768];
      HS_KEY_TYPE r0_8 = (vout + gmem_l_idx)[896];
      HS_KEY_TYPE r0_9 = (vout + gmem_l_idx)[1024];
      HS_KEY_TYPE r0_10 = (vout + gmem_l_idx)[1152];
      HS_KEY_TYPE r0_11 = (vout + gmem_l_idx)[1280];
      HS_KEY_TYPE r0_12 = (vout + gmem_l_idx)[1408];
      HS_KEY_TYPE r0_13 = (vout + gmem_l_idx)[1536];
      HS_KEY_TYPE r0_14 = (vout + gmem_l_idx)[1664];
      HS_KEY_TYPE r0_15 = (vout + gmem_l_idx)[1792];
      HS_KEY_TYPE r0_16 = (vout + gmem_l_idx)[1920];
      HS_CMP_XCHG(r0_1, r0_9)
      HS_CMP_XCHG(r0_5, r0_13)
      HS_CMP_XCHG(r0_1, r0_5)
      HS_CMP_XCHG(r0_9, r0_13)
      HS_CMP_XCHG(r0_3, r0_11)
      HS_CMP_XCHG(r0_7, r0_15)
      HS_CMP_XCHG(r0_3, r0_7)
      HS_CMP_XCHG(r0_11, r0_15)
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_5, r0_7)
      HS_CMP_XCHG(r0_9, r0_11)
      HS_CMP_XCHG(r0_13, r0_15)
      HS_CMP_XCHG(r0_2, r0_10)
      HS_CMP_XCHG(r0_6, r0_14)
      HS_CMP_XCHG(r0_2, r0_6)
      HS_CMP_XCHG(r0_10, r0_14)
      HS_CMP_XCHG(r0_4, r0_12)
      HS_CMP_XCHG(r0_8, r0_16)
      HS_CMP_XCHG(r0_4, r0_8)
      HS_CMP_XCHG(r0_12, r0_16)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_6, r0_8)
      HS_CMP_XCHG(r0_10, r0_12)
      HS_CMP_XCHG(r0_14, r0_16)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      HS_CMP_XCHG(r0_5, r0_6)
      HS_CMP_XCHG(r0_7, r0_8)
      HS_CMP_XCHG(r0_9, r0_10)
      HS_CMP_XCHG(r0_11, r0_12)
      HS_CMP_XCHG(r0_13, r0_14)
      HS_CMP_XCHG(r0_15, r0_16)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_l_idx)[8] = r0_2;
      (shared.m + smem_l_idx)[16] = r0_3;
      (shared.m + smem_l_idx)[24] = r0_4;
      (shared.m + smem_l_idx)[32] = r0_5;
      (shared.m + smem_l_idx)[40] = r0_6;
      (shared.m + smem_l_idx)[48] = r0_7;
      (shared.m + smem_l_idx)[56] = r0_8;
      (shared.m + smem_l_idx)[64] = r0_9;
      (shared.m + smem_l_idx)[72] = r0_10;
      (shared.m + smem_l_idx)[80] = r0_11;
      (shared.m + smem_l_idx)[88] = r0_12;
      (shared.m + smem_l_idx)[96] = r0_13;
      (shared.m + smem_l_idx)[104] = r0_14;
      (shared.m + smem_l_idx)[112] = r0_15;
      (shared.m + smem_l_idx)[120] = r0_16;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  HS_KEY_TYPE r1 = (shared.m + get_local_id(0))[16 * 8 * 0];
  HS_KEY_TYPE r2 = (shared.m + get_local_id(0))[16 * 8 * 1];
  HS_KEY_TYPE r3 = (shared.m + get_local_id(0))[16 * 8 * 2];
  HS_KEY_TYPE r4 = (shared.m + get_local_id(0))[16 * 8 * 3];
  HS_KEY_TYPE r5 = (shared.m + get_local_id(0))[16 * 8 * 4];
  HS_KEY_TYPE r6 = (shared.m + get_local_id(0))[16 * 8 * 5];
  HS_KEY_TYPE r7 = (shared.m + get_local_id(0))[16 * 8 * 6];
  HS_KEY_TYPE r8 = (shared.m + get_local_id(0))[16 * 8 * 7];
  HS_KEY_TYPE r9 = (shared.m + get_local_id(0))[16 * 8 * 8];
  HS_KEY_TYPE r10 = (shared.m + get_local_id(0))[16 * 8 * 9];
  HS_KEY_TYPE r11 = (shared.m + get_local_id(0))[16 * 8 * 10];
  HS_KEY_TYPE r12 = (shared.m + get_local_id(0))[16 * 8 * 11];
  HS_KEY_TYPE r13 = (shared.m + get_local_id(0))[16 * 8 * 12];
  HS_KEY_TYPE r14 = (shared.m + get_local_id(0))[16 * 8 * 13];
  HS_KEY_TYPE r15 = (shared.m + get_local_id(0))[16 * 8 * 14];
  HS_KEY_TYPE r16 = (shared.m + get_local_id(0))[16 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bc_3(__global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 64];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  uint const gmem_l_idx = (global_id / 64) * 1024 + (global_id & 63);
  uint const smem_l_idx = get_sub_group_id() * 64 + get_sub_group_local_id();
  {
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[128];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[256];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[384];
      HS_KEY_TYPE r0_5 = (vout + gmem_l_idx)[512];
      HS_KEY_TYPE r0_6 = (vout + gmem_l_idx)[640];
      HS_KEY_TYPE r0_7 = (vout + gmem_l_idx)[768];
      HS_KEY_TYPE r0_8 = (vout + gmem_l_idx)[896];
      HS_CMP_XCHG(r0_1, r0_5)
      HS_CMP_XCHG(r0_3, r0_7)
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_5, r0_7)
      HS_CMP_XCHG(r0_2, r0_6)
      HS_CMP_XCHG(r0_4, r0_8)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_6, r0_8)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      HS_CMP_XCHG(r0_5, r0_6)
      HS_CMP_XCHG(r0_7, r0_8)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_l_idx)[8] = r0_2;
      (shared.m + smem_l_idx)[16] = r0_3;
      (shared.m + smem_l_idx)[24] = r0_4;
      (shared.m + smem_l_idx)[32] = r0_5;
      (shared.m + smem_l_idx)[40] = r0_6;
      (shared.m + smem_l_idx)[48] = r0_7;
      (shared.m + smem_l_idx)[56] = r0_8;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[64];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[192];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[320];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[448];
      HS_KEY_TYPE r0_5 = (vout + gmem_l_idx)[576];
      HS_KEY_TYPE r0_6 = (vout + gmem_l_idx)[704];
      HS_KEY_TYPE r0_7 = (vout + gmem_l_idx)[832];
      HS_KEY_TYPE r0_8 = (vout + gmem_l_idx)[960];
      HS_CMP_XCHG(r0_1, r0_5)
      HS_CMP_XCHG(r0_3, r0_7)
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_5, r0_7)
      HS_CMP_XCHG(r0_2, r0_6)
      HS_CMP_XCHG(r0_4, r0_8)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_6, r0_8)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      HS_CMP_XCHG(r0_5, r0_6)
      HS_CMP_XCHG(r0_7, r0_8)
      (shared.m + smem_l_idx)[512] = r0_1;
      (shared.m + smem_l_idx)[520] = r0_2;
      (shared.m + smem_l_idx)[528] = r0_3;
      (shared.m + smem_l_idx)[536] = r0_4;
      (shared.m + smem_l_idx)[544] = r0_5;
      (shared.m + smem_l_idx)[552] = r0_6;
      (shared.m + smem_l_idx)[560] = r0_7;
      (shared.m + smem_l_idx)[568] = r0_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  HS_KEY_TYPE r1 = (shared.m + get_local_id(0))[8 * 8 * 0];
  HS_KEY_TYPE r2 = (shared.m + get_local_id(0))[8 * 8 * 1];
  HS_KEY_TYPE r3 = (shared.m + get_local_id(0))[8 * 8 * 2];
  HS_KEY_TYPE r4 = (shared.m + get_local_id(0))[8 * 8 * 3];
  HS_KEY_TYPE r5 = (shared.m + get_local_id(0))[8 * 8 * 4];
  HS_KEY_TYPE r6 = (shared.m + get_local_id(0))[8 * 8 * 5];
  HS_KEY_TYPE r7 = (shared.m + get_local_id(0))[8 * 8 * 6];
  HS_KEY_TYPE r8 = (shared.m + get_local_id(0))[8 * 8 * 7];
  HS_KEY_TYPE r9 = (shared.m + get_local_id(0))[8 * 8 * 8];
  HS_KEY_TYPE r10 = (shared.m + get_local_id(0))[8 * 8 * 9];
  HS_KEY_TYPE r11 = (shared.m + get_local_id(0))[8 * 8 * 10];
  HS_KEY_TYPE r12 = (shared.m + get_local_id(0))[8 * 8 * 11];
  HS_KEY_TYPE r13 = (shared.m + get_local_id(0))[8 * 8 * 12];
  HS_KEY_TYPE r14 = (shared.m + get_local_id(0))[8 * 8 * 13];
  HS_KEY_TYPE r15 = (shared.m + get_local_id(0))[8 * 8 * 14];
  HS_KEY_TYPE r16 = (shared.m + get_local_id(0))[8 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bc_2(__global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 32];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  uint const gmem_l_idx = (global_id / 32) * 512 + (global_id & 31);
  uint const smem_l_idx = get_sub_group_id() * 32 + get_sub_group_local_id();
  {
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[128];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[256];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[384];
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_l_idx)[8] = r0_2;
      (shared.m + smem_l_idx)[16] = r0_3;
      (shared.m + smem_l_idx)[24] = r0_4;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[32];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[160];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[288];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[416];
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      (shared.m + smem_l_idx)[128] = r0_1;
      (shared.m + smem_l_idx)[136] = r0_2;
      (shared.m + smem_l_idx)[144] = r0_3;
      (shared.m + smem_l_idx)[152] = r0_4;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[64];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[192];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[320];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[448];
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      (shared.m + smem_l_idx)[256] = r0_1;
      (shared.m + smem_l_idx)[264] = r0_2;
      (shared.m + smem_l_idx)[272] = r0_3;
      (shared.m + smem_l_idx)[280] = r0_4;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[96];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[224];
      HS_KEY_TYPE r0_3 = (vout + gmem_l_idx)[352];
      HS_KEY_TYPE r0_4 = (vout + gmem_l_idx)[480];
      HS_CMP_XCHG(r0_1, r0_3)
      HS_CMP_XCHG(r0_2, r0_4)
      HS_CMP_XCHG(r0_1, r0_2)
      HS_CMP_XCHG(r0_3, r0_4)
      (shared.m + smem_l_idx)[384] = r0_1;
      (shared.m + smem_l_idx)[392] = r0_2;
      (shared.m + smem_l_idx)[400] = r0_3;
      (shared.m + smem_l_idx)[408] = r0_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  HS_KEY_TYPE r1 = (shared.m + get_local_id(0))[4 * 8 * 0];
  HS_KEY_TYPE r2 = (shared.m + get_local_id(0))[4 * 8 * 1];
  HS_KEY_TYPE r3 = (shared.m + get_local_id(0))[4 * 8 * 2];
  HS_KEY_TYPE r4 = (shared.m + get_local_id(0))[4 * 8 * 3];
  HS_KEY_TYPE r5 = (shared.m + get_local_id(0))[4 * 8 * 4];
  HS_KEY_TYPE r6 = (shared.m + get_local_id(0))[4 * 8 * 5];
  HS_KEY_TYPE r7 = (shared.m + get_local_id(0))[4 * 8 * 6];
  HS_KEY_TYPE r8 = (shared.m + get_local_id(0))[4 * 8 * 7];
  HS_KEY_TYPE r9 = (shared.m + get_local_id(0))[4 * 8 * 8];
  HS_KEY_TYPE r10 = (shared.m + get_local_id(0))[4 * 8 * 9];
  HS_KEY_TYPE r11 = (shared.m + get_local_id(0))[4 * 8 * 10];
  HS_KEY_TYPE r12 = (shared.m + get_local_id(0))[4 * 8 * 11];
  HS_KEY_TYPE r13 = (shared.m + get_local_id(0))[4 * 8 * 12];
  HS_KEY_TYPE r14 = (shared.m + get_local_id(0))[4 * 8 * 13];
  HS_KEY_TYPE r15 = (shared.m + get_local_id(0))[4 * 8 * 14];
  HS_KEY_TYPE r16 = (shared.m + get_local_id(0))[4 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bc_1(__global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
    HS_KEY_TYPE m[16 * 16];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  uint const gmem_l_idx = (global_id / 16) * 256 + (global_id & 15);
  uint const smem_l_idx = get_sub_group_id() * 16 + get_sub_group_local_id();
  {
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[0];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[128];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[0] = r0_1;
      (shared.m + smem_l_idx)[8] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[16];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[144];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[32] = r0_1;
      (shared.m + smem_l_idx)[40] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[32];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[160];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[64] = r0_1;
      (shared.m + smem_l_idx)[72] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[48];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[176];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[96] = r0_1;
      (shared.m + smem_l_idx)[104] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[64];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[192];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[128] = r0_1;
      (shared.m + smem_l_idx)[136] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[80];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[208];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[160] = r0_1;
      (shared.m + smem_l_idx)[168] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[96];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[224];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[192] = r0_1;
      (shared.m + smem_l_idx)[200] = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = (vout + gmem_l_idx)[112];
      HS_KEY_TYPE r0_2 = (vout + gmem_l_idx)[240];
      HS_CMP_XCHG(r0_1, r0_2)
      (shared.m + smem_l_idx)[224] = r0_1;
      (shared.m + smem_l_idx)[232] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  HS_KEY_TYPE r1 = (shared.m + get_local_id(0))[2 * 8 * 0];
  HS_KEY_TYPE r2 = (shared.m + get_local_id(0))[2 * 8 * 1];
  HS_KEY_TYPE r3 = (shared.m + get_local_id(0))[2 * 8 * 2];
  HS_KEY_TYPE r4 = (shared.m + get_local_id(0))[2 * 8 * 3];
  HS_KEY_TYPE r5 = (shared.m + get_local_id(0))[2 * 8 * 4];
  HS_KEY_TYPE r6 = (shared.m + get_local_id(0))[2 * 8 * 5];
  HS_KEY_TYPE r7 = (shared.m + get_local_id(0))[2 * 8 * 6];
  HS_KEY_TYPE r8 = (shared.m + get_local_id(0))[2 * 8 * 7];
  HS_KEY_TYPE r9 = (shared.m + get_local_id(0))[2 * 8 * 8];
  HS_KEY_TYPE r10 = (shared.m + get_local_id(0))[2 * 8 * 9];
  HS_KEY_TYPE r11 = (shared.m + get_local_id(0))[2 * 8 * 10];
  HS_KEY_TYPE r12 = (shared.m + get_local_id(0))[2 * 8 * 11];
  HS_KEY_TYPE r13 = (shared.m + get_local_id(0))[2 * 8 * 12];
  HS_KEY_TYPE r14 = (shared.m + get_local_id(0))[2 * 8 * 13];
  HS_KEY_TYPE r15 = (shared.m + get_local_id(0))[2 * 8 * 14];
  HS_KEY_TYPE r16 = (shared.m + get_local_id(0))[2 * 8 * 15];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_bc_0(__global HS_KEY_TYPE* const restrict vout)
{
  __local union
  {
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_idx = (global_id / 8) * 128 + (global_id & 7);

  HS_KEY_TYPE r1 = (vout + gmem_idx)[0 * 8];
  HS_KEY_TYPE r2 = (vout + gmem_idx)[1 * 8];
  HS_KEY_TYPE r3 = (vout + gmem_idx)[2 * 8];
  HS_KEY_TYPE r4 = (vout + gmem_idx)[3 * 8];
  HS_KEY_TYPE r5 = (vout + gmem_idx)[4 * 8];
  HS_KEY_TYPE r6 = (vout + gmem_idx)[5 * 8];
  HS_KEY_TYPE r7 = (vout + gmem_idx)[6 * 8];
  HS_KEY_TYPE r8 = (vout + gmem_idx)[7 * 8];
  HS_KEY_TYPE r9 = (vout + gmem_idx)[8 * 8];
  HS_KEY_TYPE r10 = (vout + gmem_idx)[9 * 8];
  HS_KEY_TYPE r11 = (vout + gmem_idx)[10 * 8];
  HS_KEY_TYPE r12 = (vout + gmem_idx)[11 * 8];
  HS_KEY_TYPE r13 = (vout + gmem_idx)[12 * 8];
  HS_KEY_TYPE r14 = (vout + gmem_idx)[13 * 8];
  HS_KEY_TYPE r15 = (vout + gmem_idx)[14 * 8];
  HS_KEY_TYPE r16 = (vout + gmem_idx)[15 * 8];
  { { uint const half_lane_mask = 4;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 2;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
{
  uint const half_lane_mask = 1;
  uint const half_lane_idx = get_sub_group_local_id() ^ half_lane_mask;
  int const t_lt = get_sub_group_local_id() < half_lane_idx;
  HS_CMP_HALF(0, r1)
  HS_CMP_HALF(1, r2)
  HS_CMP_HALF(2, r3)
  HS_CMP_HALF(3, r4)
  HS_CMP_HALF(4, r5)
  HS_CMP_HALF(5, r6)
  HS_CMP_HALF(6, r7)
  HS_CMP_HALF(7, r8)
  HS_CMP_HALF(8, r9)
  HS_CMP_HALF(9, r10)
  HS_CMP_HALF(10, r11)
  HS_CMP_HALF(11, r12)
  HS_CMP_HALF(12, r13)
  HS_CMP_HALF(13, r14)
  HS_CMP_HALF(14, r15)
  HS_CMP_HALF(15, r16)
}
HS_CMP_XCHG(r1, r9)
HS_CMP_XCHG(r5, r13)
HS_CMP_XCHG(r1, r5)
HS_CMP_XCHG(r9, r13)
HS_CMP_XCHG(r3, r11)
HS_CMP_XCHG(r7, r15)
HS_CMP_XCHG(r3, r7)
HS_CMP_XCHG(r11, r15)
HS_CMP_XCHG(r1, r3)
HS_CMP_XCHG(r5, r7)
HS_CMP_XCHG(r9, r11)
HS_CMP_XCHG(r13, r15)
HS_CMP_XCHG(r2, r10)
HS_CMP_XCHG(r6, r14)
HS_CMP_XCHG(r2, r6)
HS_CMP_XCHG(r10, r14)
HS_CMP_XCHG(r4, r12)
HS_CMP_XCHG(r8, r16)
HS_CMP_XCHG(r4, r8)
HS_CMP_XCHG(r12, r16)
HS_CMP_XCHG(r2, r4)
HS_CMP_XCHG(r6, r8)
HS_CMP_XCHG(r10, r12)
HS_CMP_XCHG(r14, r16)
HS_CMP_XCHG(r1, r2)
HS_CMP_XCHG(r3, r4)
HS_CMP_XCHG(r5, r6)
HS_CMP_XCHG(r7, r8)
HS_CMP_XCHG(r9, r10)
HS_CMP_XCHG(r11, r12)
HS_CMP_XCHG(r13, r14)
HS_CMP_XCHG(r15, r16)
}
(vout + gmem_idx)[0 * 8] = r1;
(vout + gmem_idx)[1 * 8] = r2;
(vout + gmem_idx)[2 * 8] = r3;
(vout + gmem_idx)[3 * 8] = r4;
(vout + gmem_idx)[4 * 8] = r5;
(vout + gmem_idx)[5 * 8] = r6;
(vout + gmem_idx)[6 * 8] = r7;
(vout + gmem_idx)[7 * 8] = r8;
(vout + gmem_idx)[8 * 8] = r9;
(vout + gmem_idx)[9 * 8] = r10;
(vout + gmem_idx)[10 * 8] = r11;
(vout + gmem_idx)[11 * 8] = r12;
(vout + gmem_idx)[12 * 8] = r13;
(vout + gmem_idx)[13 * 8] = r14;
(vout + gmem_idx)[14 * 8] = r15;
(vout + gmem_idx)[15 * 8] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_1(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 0;

  uint const merge_stride = 16 * 8 << 0;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 0)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_2(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 1;

  uint const merge_stride = 16 * 8 << 1;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 1)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_3(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 2;

  uint const merge_stride = 16 * 8 << 2;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 2)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_4(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 3;

  uint const merge_stride = 16 * 8 << 3;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 3)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_5(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 4;

  uint const merge_stride = 16 * 8 << 4;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 4)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_6(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 5;

  uint const merge_stride = 16 * 8 << 5;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 5)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_5(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 0;

  uint const merge_stride = 16 * 8 << 0;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 0)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_7(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 6;

  uint const merge_stride = 16 * 8 << 6;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 6)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_6(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 1;

  uint const merge_stride = 16 * 8 << 1;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 1)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_8(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 7;

  uint const merge_stride = 16 * 8 << 7;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 7)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_7(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 2;

  uint const merge_stride = 16 * 8 << 2;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 2)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_9(__global HS_KEY_TYPE* const restrict vout,
               uint const fm_full,
               uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 8;

  uint const merge_stride = 16 * 8 << 8;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 8)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_8(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 3;

  uint const merge_stride = 16 * 8 << 3;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 3)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_10(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 9;

  uint const merge_stride = 16 * 8 << 9;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 9)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_9(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 4;

  uint const merge_stride = 16 * 8 << 4;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 4)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_11(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 10;

  uint const merge_stride = 16 * 8 << 10;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 10)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_10(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 5;

  uint const merge_stride = 16 * 8 << 5;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 5)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_12(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 11;

  uint const merge_stride = 16 * 8 << 11;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 11)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_11(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 6;

  uint const merge_stride = 16 * 8 << 6;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 6)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_13(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 12;

  uint const merge_stride = 16 * 8 << 12;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 12)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_12(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 7;

  uint const merge_stride = 16 * 8 << 7;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 7)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_14(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 13;

  uint const merge_stride = 16 * 8 << 13;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 13)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_13(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 8;

  uint const merge_stride = 16 * 8 << 8;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 8)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_15(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 14;

  uint const merge_stride = 16 * 8 << 14;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 14)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_14(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 9;

  uint const merge_stride = 16 * 8 << 9;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 9)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_fm_16(__global HS_KEY_TYPE* const restrict vout,
                uint const fm_full,
                uint const fm_frac)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = warp_idx / 16 >> 15;

  uint const merge_stride = 16 * 8 << 15;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;

  uint const merge_l_off =
    (warp_idx - merge_idx * (16 << 15)) * 8 + warp_lane_idx;
  uint const merge_l_end = merge_stride * (32 / 2 - 1) + merge_l_off;

  int const merge_r_off = merge_keys - merge_l_end - 1;

  __global HS_KEY_TYPE* const restrict merge_l =
    vout + (merge_base + merge_l_off);
  __global HS_KEY_TYPE* const restrict merge_r =
    vout + (merge_base + merge_r_off);

  HS_KEY_TYPE r1 = merge_l[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_l[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_l[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_l[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_l[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_l[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_l[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_l[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_l[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_l[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_l[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_l[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_l[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_l[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_l[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_l[15 * merge_stride];
  if (merge_idx < fm_full) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_KEY_TYPE r25 = merge_r[8 * merge_stride];
    HS_KEY_TYPE r26 = merge_r[9 * merge_stride];
    HS_KEY_TYPE r27 = merge_r[10 * merge_stride];
    HS_KEY_TYPE r28 = merge_r[11 * merge_stride];
    HS_KEY_TYPE r29 = merge_r[12 * merge_stride];
    HS_KEY_TYPE r30 = merge_r[13 * merge_stride];
    HS_KEY_TYPE r31 = merge_r[14 * merge_stride];
    HS_KEY_TYPE r32 = merge_r[15 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r8, r25)
    HS_CMP_XCHG(r7, r26)
    HS_CMP_XCHG(r6, r27)
    HS_CMP_XCHG(r5, r28)
    HS_CMP_XCHG(r4, r29)
    HS_CMP_XCHG(r3, r30)
    HS_CMP_XCHG(r2, r31)
    HS_CMP_XCHG(r1, r32)
    HS_CMP_XCHG(r17, r25)
    HS_CMP_XCHG(r21, r29)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r25, r29)
    HS_CMP_XCHG(r19, r27)
    HS_CMP_XCHG(r23, r31)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r27, r31)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r25, r27)
    HS_CMP_XCHG(r29, r31)
    HS_CMP_XCHG(r18, r26)
    HS_CMP_XCHG(r22, r30)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r26, r30)
    HS_CMP_XCHG(r20, r28)
    HS_CMP_XCHG(r24, r32)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r28, r32)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r26, r28)
    HS_CMP_XCHG(r30, r32)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    HS_CMP_XCHG(r25, r26)
    HS_CMP_XCHG(r27, r28)
    HS_CMP_XCHG(r29, r30)
    HS_CMP_XCHG(r31, r32)
    merge_r[15 * merge_stride] = r32;
    merge_r[14 * merge_stride] = r31;
    merge_r[13 * merge_stride] = r30;
    merge_r[12 * merge_stride] = r29;
    merge_r[11 * merge_stride] = r28;
    merge_r[10 * merge_stride] = r27;
    merge_r[9 * merge_stride] = r26;
    merge_r[8 * merge_stride] = r25;
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 8) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_KEY_TYPE r21 = merge_r[4 * merge_stride];
    HS_KEY_TYPE r22 = merge_r[5 * merge_stride];
    HS_KEY_TYPE r23 = merge_r[6 * merge_stride];
    HS_KEY_TYPE r24 = merge_r[7 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r12, r21)
    HS_CMP_XCHG(r11, r22)
    HS_CMP_XCHG(r10, r23)
    HS_CMP_XCHG(r9, r24)
    HS_CMP_XCHG(r17, r21)
    HS_CMP_XCHG(r19, r23)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r21, r23)
    HS_CMP_XCHG(r18, r22)
    HS_CMP_XCHG(r20, r24)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r22, r24)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    HS_CMP_XCHG(r21, r22)
    HS_CMP_XCHG(r23, r24)
    merge_r[7 * merge_stride] = r24;
    merge_r[6 * merge_stride] = r23;
    merge_r[5 * merge_stride] = r22;
    merge_r[4 * merge_stride] = r21;
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 4) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_KEY_TYPE r19 = merge_r[2 * merge_stride];
    HS_KEY_TYPE r20 = merge_r[3 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r14, r19)
    HS_CMP_XCHG(r13, r20)
    HS_CMP_XCHG(r17, r19)
    HS_CMP_XCHG(r18, r20)
    HS_CMP_XCHG(r17, r18)
    HS_CMP_XCHG(r19, r20)
    merge_r[3 * merge_stride] = r20;
    merge_r[2 * merge_stride] = r19;
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else if (fm_frac == 2) {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_KEY_TYPE r18 = merge_r[1 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    HS_CMP_XCHG(r15, r18)
    HS_CMP_XCHG(r17, r18)
    merge_r[1 * merge_stride] = r18;
    merge_r[0 * merge_stride] = r17;
  } else {
    HS_KEY_TYPE r17 = merge_r[0 * merge_stride];
    HS_CMP_XCHG(r16, r17)
    merge_r[0 * merge_stride] = r17;
  }
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  merge_l[15 * merge_stride] = r16;
  merge_l[14 * merge_stride] = r15;
  merge_l[13 * merge_stride] = r14;
  merge_l[12 * merge_stride] = r13;
  merge_l[11 * merge_stride] = r12;
  merge_l[10 * merge_stride] = r11;
  merge_l[9 * merge_stride] = r10;
  merge_l[8 * merge_stride] = r9;
  merge_l[7 * merge_stride] = r8;
  merge_l[6 * merge_stride] = r7;
  merge_l[5 * merge_stride] = r6;
  merge_l[4 * merge_stride] = r5;
  merge_l[3 * merge_stride] = r4;
  merge_l[2 * merge_stride] = r3;
  merge_l[1 * merge_stride] = r2;
  merge_l[0 * merge_stride] = r1;
}

__kernel __attribute__((intel_reqd_sub_group_size(8))) void
hs_kernel_hm_15(__global HS_KEY_TYPE* const restrict vout)
{
  uint const global_id = (uint)get_global_id(0);
  uint const warp_idx = global_id / 8;
  uint const warp_lane_idx = global_id & 7;

  uint const merge_idx = (warp_idx / 16) >> 10;

  uint const merge_stride = 16 * 8 << 10;
  uint const merge_keys = merge_stride * 32;

  uint const merge_base = merge_idx * merge_keys;
  uint const merge_off = (warp_idx - merge_idx * (16 << 10)) * 8;

  __global HS_KEY_TYPE* const restrict merge_ptr =
    vout + (merge_base + merge_off + warp_lane_idx);

  HS_KEY_TYPE r1 = merge_ptr[0 * merge_stride];
  HS_KEY_TYPE r2 = merge_ptr[1 * merge_stride];
  HS_KEY_TYPE r3 = merge_ptr[2 * merge_stride];
  HS_KEY_TYPE r4 = merge_ptr[3 * merge_stride];
  HS_KEY_TYPE r5 = merge_ptr[4 * merge_stride];
  HS_KEY_TYPE r6 = merge_ptr[5 * merge_stride];
  HS_KEY_TYPE r7 = merge_ptr[6 * merge_stride];
  HS_KEY_TYPE r8 = merge_ptr[7 * merge_stride];
  HS_KEY_TYPE r9 = merge_ptr[8 * merge_stride];
  HS_KEY_TYPE r10 = merge_ptr[9 * merge_stride];
  HS_KEY_TYPE r11 = merge_ptr[10 * merge_stride];
  HS_KEY_TYPE r12 = merge_ptr[11 * merge_stride];
  HS_KEY_TYPE r13 = merge_ptr[12 * merge_stride];
  HS_KEY_TYPE r14 = merge_ptr[13 * merge_stride];
  HS_KEY_TYPE r15 = merge_ptr[14 * merge_stride];
  HS_KEY_TYPE r16 = merge_ptr[15 * merge_stride];
  HS_KEY_TYPE r17 = merge_ptr[16 * merge_stride];
  HS_KEY_TYPE r18 = merge_ptr[17 * merge_stride];
  HS_KEY_TYPE r19 = merge_ptr[18 * merge_stride];
  HS_KEY_TYPE r20 = merge_ptr[19 * merge_stride];
  HS_KEY_TYPE r21 = merge_ptr[20 * merge_stride];
  HS_KEY_TYPE r22 = merge_ptr[21 * merge_stride];
  HS_KEY_TYPE r23 = merge_ptr[22 * merge_stride];
  HS_KEY_TYPE r24 = merge_ptr[23 * merge_stride];
  HS_KEY_TYPE r25 = merge_ptr[24 * merge_stride];
  HS_KEY_TYPE r26 = merge_ptr[25 * merge_stride];
  HS_KEY_TYPE r27 = merge_ptr[26 * merge_stride];
  HS_KEY_TYPE r28 = merge_ptr[27 * merge_stride];
  HS_KEY_TYPE r29 = merge_ptr[28 * merge_stride];
  HS_KEY_TYPE r30 = merge_ptr[29 * merge_stride];
  HS_KEY_TYPE r31 = merge_ptr[30 * merge_stride];
  HS_KEY_TYPE r32 = merge_ptr[31 * merge_stride];
  HS_CMP_XCHG(r1, r17)
  HS_CMP_XCHG(r9, r25)
  HS_CMP_XCHG(r1, r9)
  HS_CMP_XCHG(r17, r25)
  HS_CMP_XCHG(r5, r21)
  HS_CMP_XCHG(r13, r29)
  HS_CMP_XCHG(r5, r13)
  HS_CMP_XCHG(r21, r29)
  HS_CMP_XCHG(r1, r5)
  HS_CMP_XCHG(r9, r13)
  HS_CMP_XCHG(r17, r21)
  HS_CMP_XCHG(r25, r29)
  HS_CMP_XCHG(r3, r19)
  HS_CMP_XCHG(r11, r27)
  HS_CMP_XCHG(r3, r11)
  HS_CMP_XCHG(r19, r27)
  HS_CMP_XCHG(r7, r23)
  HS_CMP_XCHG(r15, r31)
  HS_CMP_XCHG(r7, r15)
  HS_CMP_XCHG(r23, r31)
  HS_CMP_XCHG(r3, r7)
  HS_CMP_XCHG(r11, r15)
  HS_CMP_XCHG(r19, r23)
  HS_CMP_XCHG(r27, r31)
  HS_CMP_XCHG(r1, r3)
  HS_CMP_XCHG(r5, r7)
  HS_CMP_XCHG(r9, r11)
  HS_CMP_XCHG(r13, r15)
  HS_CMP_XCHG(r17, r19)
  HS_CMP_XCHG(r21, r23)
  HS_CMP_XCHG(r25, r27)
  HS_CMP_XCHG(r29, r31)
  HS_CMP_XCHG(r2, r18)
  HS_CMP_XCHG(r10, r26)
  HS_CMP_XCHG(r2, r10)
  HS_CMP_XCHG(r18, r26)
  HS_CMP_XCHG(r6, r22)
  HS_CMP_XCHG(r14, r30)
  HS_CMP_XCHG(r6, r14)
  HS_CMP_XCHG(r22, r30)
  HS_CMP_XCHG(r2, r6)
  HS_CMP_XCHG(r10, r14)
  HS_CMP_XCHG(r18, r22)
  HS_CMP_XCHG(r26, r30)
  HS_CMP_XCHG(r4, r20)
  HS_CMP_XCHG(r12, r28)
  HS_CMP_XCHG(r4, r12)
  HS_CMP_XCHG(r20, r28)
  HS_CMP_XCHG(r8, r24)
  HS_CMP_XCHG(r16, r32)
  HS_CMP_XCHG(r8, r16)
  HS_CMP_XCHG(r24, r32)
  HS_CMP_XCHG(r4, r8)
  HS_CMP_XCHG(r12, r16)
  HS_CMP_XCHG(r20, r24)
  HS_CMP_XCHG(r28, r32)
  HS_CMP_XCHG(r2, r4)
  HS_CMP_XCHG(r6, r8)
  HS_CMP_XCHG(r10, r12)
  HS_CMP_XCHG(r14, r16)
  HS_CMP_XCHG(r18, r20)
  HS_CMP_XCHG(r22, r24)
  HS_CMP_XCHG(r26, r28)
  HS_CMP_XCHG(r30, r32)
  HS_CMP_XCHG(r1, r2)
  HS_CMP_XCHG(r3, r4)
  HS_CMP_XCHG(r5, r6)
  HS_CMP_XCHG(r7, r8)
  HS_CMP_XCHG(r9, r10)
  HS_CMP_XCHG(r11, r12)
  HS_CMP_XCHG(r13, r14)
  HS_CMP_XCHG(r15, r16)
  HS_CMP_XCHG(r17, r18)
  HS_CMP_XCHG(r19, r20)
  HS_CMP_XCHG(r21, r22)
  HS_CMP_XCHG(r23, r24)
  HS_CMP_XCHG(r25, r26)
  HS_CMP_XCHG(r27, r28)
  HS_CMP_XCHG(r29, r30)
  HS_CMP_XCHG(r31, r32)
  merge_ptr[31 * merge_stride] = r32;
  merge_ptr[30 * merge_stride] = r31;
  merge_ptr[29 * merge_stride] = r30;
  merge_ptr[28 * merge_stride] = r29;
  merge_ptr[27 * merge_stride] = r28;
  merge_ptr[26 * merge_stride] = r27;
  merge_ptr[25 * merge_stride] = r26;
  merge_ptr[24 * merge_stride] = r25;
  merge_ptr[23 * merge_stride] = r24;
  merge_ptr[22 * merge_stride] = r23;
  merge_ptr[21 * merge_stride] = r22;
  merge_ptr[20 * merge_stride] = r21;
  merge_ptr[19 * merge_stride] = r20;
  merge_ptr[18 * merge_stride] = r19;
  merge_ptr[17 * merge_stride] = r18;
  merge_ptr[16 * merge_stride] = r17;
  merge_ptr[15 * merge_stride] = r16;
  merge_ptr[14 * merge_stride] = r15;
  merge_ptr[13 * merge_stride] = r14;
  merge_ptr[12 * merge_stride] = r13;
  merge_ptr[11 * merge_stride] = r12;
  merge_ptr[10 * merge_stride] = r11;
  merge_ptr[9 * merge_stride] = r10;
  merge_ptr[8 * merge_stride] = r9;
  merge_ptr[7 * merge_stride] = r8;
  merge_ptr[6 * merge_stride] = r7;
  merge_ptr[5 * merge_stride] = r6;
  merge_ptr[4 * merge_stride] = r5;
  merge_ptr[3 * merge_stride] = r4;
  merge_ptr[2 * merge_stride] = r3;
  merge_ptr[1 * merge_stride] = r2;
  merge_ptr[0 * merge_stride] = r1;
}

//
//
//
