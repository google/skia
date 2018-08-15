

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 1, 1, 1))) void
hs_kernel_bs_0(__global ulong const* const restrict vin,
               __global ulong* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vin[gmem_idx + (1 << 3) * 0];
  ulong r2 = vin[gmem_idx + (1 << 3) * 1];
  ulong r3 = vin[gmem_idx + (1 << 3) * 2];
  ulong r4 = vin[gmem_idx + (1 << 3) * 3];
  ulong r5 = vin[gmem_idx + (1 << 3) * 4];
  ulong r6 = vin[gmem_idx + (1 << 3) * 5];
  ulong r7 = vin[gmem_idx + (1 << 3) * 6];
  ulong r8 = vin[gmem_idx + (1 << 3) * 7];
  ulong r9 = vin[gmem_idx + (1 << 3) * 8];
  ulong r10 = vin[gmem_idx + (1 << 3) * 9];
  ulong r11 = vin[gmem_idx + (1 << 3) * 10];
  ulong r12 = vin[gmem_idx + (1 << 3) * 11];
  ulong r13 = vin[gmem_idx + (1 << 3) * 12];
  ulong r14 = vin[gmem_idx + (1 << 3) * 13];
  ulong r15 = vin[gmem_idx + (1 << 3) * 14];
  ulong r16 = vin[gmem_idx + (1 << 3) * 15];
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r6 >= r11) {
    ulong const t = r6;
    r6 = r11;
    r11 = t;
  };
  if (r7 >= r10) {
    ulong const t = r7;
    r7 = r10;
    r10 = t;
  };
  if (r4 >= r13) {
    ulong const t = r4;
    r4 = r13;
    r13 = t;
  };
  if (r14 >= r15) {
    ulong const t = r14;
    r14 = r15;
    r15 = t;
  };
  if (r8 >= r12) {
    ulong const t = r8;
    r8 = r12;
    r12 = t;
  };
  if (r2 >= r3) {
    ulong const t = r2;
    r2 = r3;
    r3 = t;
  };
  if (r5 >= r9) {
    ulong const t = r5;
    r5 = r9;
    r9 = t;
  };
  if (r2 >= r5) {
    ulong const t = r2;
    r2 = r5;
    r5 = t;
  };
  if (r8 >= r14) {
    ulong const t = r8;
    r8 = r14;
    r14 = t;
  };
  if (r3 >= r9) {
    ulong const t = r3;
    r3 = r9;
    r9 = t;
  };
  if (r12 >= r15) {
    ulong const t = r12;
    r12 = r15;
    r15 = t;
  };
  if (r3 >= r5) {
    ulong const t = r3;
    r3 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r14) {
    ulong const t = r12;
    r12 = r14;
    r14 = t;
  };
  if (r4 >= r9) {
    ulong const t = r4;
    r4 = r9;
    r9 = t;
  };
  if (r8 >= r13) {
    ulong const t = r8;
    r8 = r13;
    r13 = t;
  };
  if (r7 >= r9) {
    ulong const t = r7;
    r7 = r9;
    r9 = t;
  };
  if (r11 >= r13) {
    ulong const t = r11;
    r11 = r13;
    r13 = t;
  };
  if (r4 >= r6) {
    ulong const t = r4;
    r4 = r6;
    r6 = t;
  };
  if (r8 >= r10) {
    ulong const t = r8;
    r8 = r10;
    r10 = t;
  };
  if (r4 >= r5) {
    ulong const t = r4;
    r4 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r8 >= r9) {
    ulong const t = r8;
    r8 = r9;
    r9 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r13) {
    ulong const t = r12;
    r12 = r13;
    r13 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 2, 1, 1))) void
hs_kernel_bs_1(__global ulong const* const restrict vin,
               __global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[16 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vin[gmem_idx + (1 << 3) * 0];
  ulong r2 = vin[gmem_idx + (1 << 3) * 1];
  ulong r3 = vin[gmem_idx + (1 << 3) * 2];
  ulong r4 = vin[gmem_idx + (1 << 3) * 3];
  ulong r5 = vin[gmem_idx + (1 << 3) * 4];
  ulong r6 = vin[gmem_idx + (1 << 3) * 5];
  ulong r7 = vin[gmem_idx + (1 << 3) * 6];
  ulong r8 = vin[gmem_idx + (1 << 3) * 7];
  ulong r9 = vin[gmem_idx + (1 << 3) * 8];
  ulong r10 = vin[gmem_idx + (1 << 3) * 9];
  ulong r11 = vin[gmem_idx + (1 << 3) * 10];
  ulong r12 = vin[gmem_idx + (1 << 3) * 11];
  ulong r13 = vin[gmem_idx + (1 << 3) * 12];
  ulong r14 = vin[gmem_idx + (1 << 3) * 13];
  ulong r15 = vin[gmem_idx + (1 << 3) * 14];
  ulong r16 = vin[gmem_idx + (1 << 3) * 15];
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r6 >= r11) {
    ulong const t = r6;
    r6 = r11;
    r11 = t;
  };
  if (r7 >= r10) {
    ulong const t = r7;
    r7 = r10;
    r10 = t;
  };
  if (r4 >= r13) {
    ulong const t = r4;
    r4 = r13;
    r13 = t;
  };
  if (r14 >= r15) {
    ulong const t = r14;
    r14 = r15;
    r15 = t;
  };
  if (r8 >= r12) {
    ulong const t = r8;
    r8 = r12;
    r12 = t;
  };
  if (r2 >= r3) {
    ulong const t = r2;
    r2 = r3;
    r3 = t;
  };
  if (r5 >= r9) {
    ulong const t = r5;
    r5 = r9;
    r9 = t;
  };
  if (r2 >= r5) {
    ulong const t = r2;
    r2 = r5;
    r5 = t;
  };
  if (r8 >= r14) {
    ulong const t = r8;
    r8 = r14;
    r14 = t;
  };
  if (r3 >= r9) {
    ulong const t = r3;
    r3 = r9;
    r9 = t;
  };
  if (r12 >= r15) {
    ulong const t = r12;
    r12 = r15;
    r15 = t;
  };
  if (r3 >= r5) {
    ulong const t = r3;
    r3 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r14) {
    ulong const t = r12;
    r12 = r14;
    r14 = t;
  };
  if (r4 >= r9) {
    ulong const t = r4;
    r4 = r9;
    r9 = t;
  };
  if (r8 >= r13) {
    ulong const t = r8;
    r8 = r13;
    r13 = t;
  };
  if (r7 >= r9) {
    ulong const t = r7;
    r7 = r9;
    r9 = t;
  };
  if (r11 >= r13) {
    ulong const t = r11;
    r11 = r13;
    r13 = t;
  };
  if (r4 >= r6) {
    ulong const t = r4;
    r4 = r6;
    r6 = t;
  };
  if (r8 >= r10) {
    ulong const t = r8;
    r8 = r10;
    r10 = t;
  };
  if (r4 >= r5) {
    ulong const t = r4;
    r4 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r8 >= r9) {
    ulong const t = r8;
    r8 = r9;
    r9 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r13) {
    ulong const t = r12;
    r12 = r13;
    r13 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 2) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 3) * 2) +
                          (get_sub_group_local_id() ^ ((1 << 3) - 1));
  shared.m[get_local_id(0) + (2 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (2 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_r_idx + (8)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (8)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (32)];
      ulong r0_2 = shared.m[smem_r_idx + (40)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r0_1;
      shared.m[smem_r_idx + (40)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (64)];
      ulong r0_2 = shared.m[smem_r_idx + (72)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (64)] = r0_1;
      shared.m[smem_r_idx + (72)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (96)];
      ulong r0_2 = shared.m[smem_r_idx + (104)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (96)] = r0_1;
      shared.m[smem_r_idx + (104)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (128)];
      ulong r0_2 = shared.m[smem_r_idx + (136)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_r_idx + (136)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (160)];
      ulong r0_2 = shared.m[smem_r_idx + (168)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (160)] = r0_1;
      shared.m[smem_r_idx + (168)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (192)];
      ulong r0_2 = shared.m[smem_r_idx + (200)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (192)] = r0_1;
      shared.m[smem_r_idx + (200)] = r0_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (224)];
      ulong r0_2 = shared.m[smem_r_idx + (232)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (224)] = r0_1;
      shared.m[smem_r_idx + (232)] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (2 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (2 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (2 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (2 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (2 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (2 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (2 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (2 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (2 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (2 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (2 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (2 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (2 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (2 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (2 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (2 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 4, 1, 1))) void
hs_kernel_bs_2(__global ulong const* const restrict vin,
               __global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[32 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vin[gmem_idx + (1 << 3) * 0];
  ulong r2 = vin[gmem_idx + (1 << 3) * 1];
  ulong r3 = vin[gmem_idx + (1 << 3) * 2];
  ulong r4 = vin[gmem_idx + (1 << 3) * 3];
  ulong r5 = vin[gmem_idx + (1 << 3) * 4];
  ulong r6 = vin[gmem_idx + (1 << 3) * 5];
  ulong r7 = vin[gmem_idx + (1 << 3) * 6];
  ulong r8 = vin[gmem_idx + (1 << 3) * 7];
  ulong r9 = vin[gmem_idx + (1 << 3) * 8];
  ulong r10 = vin[gmem_idx + (1 << 3) * 9];
  ulong r11 = vin[gmem_idx + (1 << 3) * 10];
  ulong r12 = vin[gmem_idx + (1 << 3) * 11];
  ulong r13 = vin[gmem_idx + (1 << 3) * 12];
  ulong r14 = vin[gmem_idx + (1 << 3) * 13];
  ulong r15 = vin[gmem_idx + (1 << 3) * 14];
  ulong r16 = vin[gmem_idx + (1 << 3) * 15];
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r6 >= r11) {
    ulong const t = r6;
    r6 = r11;
    r11 = t;
  };
  if (r7 >= r10) {
    ulong const t = r7;
    r7 = r10;
    r10 = t;
  };
  if (r4 >= r13) {
    ulong const t = r4;
    r4 = r13;
    r13 = t;
  };
  if (r14 >= r15) {
    ulong const t = r14;
    r14 = r15;
    r15 = t;
  };
  if (r8 >= r12) {
    ulong const t = r8;
    r8 = r12;
    r12 = t;
  };
  if (r2 >= r3) {
    ulong const t = r2;
    r2 = r3;
    r3 = t;
  };
  if (r5 >= r9) {
    ulong const t = r5;
    r5 = r9;
    r9 = t;
  };
  if (r2 >= r5) {
    ulong const t = r2;
    r2 = r5;
    r5 = t;
  };
  if (r8 >= r14) {
    ulong const t = r8;
    r8 = r14;
    r14 = t;
  };
  if (r3 >= r9) {
    ulong const t = r3;
    r3 = r9;
    r9 = t;
  };
  if (r12 >= r15) {
    ulong const t = r12;
    r12 = r15;
    r15 = t;
  };
  if (r3 >= r5) {
    ulong const t = r3;
    r3 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r14) {
    ulong const t = r12;
    r12 = r14;
    r14 = t;
  };
  if (r4 >= r9) {
    ulong const t = r4;
    r4 = r9;
    r9 = t;
  };
  if (r8 >= r13) {
    ulong const t = r8;
    r8 = r13;
    r13 = t;
  };
  if (r7 >= r9) {
    ulong const t = r7;
    r7 = r9;
    r9 = t;
  };
  if (r11 >= r13) {
    ulong const t = r11;
    r11 = r13;
    r13 = t;
  };
  if (r4 >= r6) {
    ulong const t = r4;
    r4 = r6;
    r6 = t;
  };
  if (r8 >= r10) {
    ulong const t = r8;
    r8 = r10;
    r10 = t;
  };
  if (r4 >= r5) {
    ulong const t = r4;
    r4 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r8 >= r9) {
    ulong const t = r8;
    r8 = r9;
    r9 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r13) {
    ulong const t = r12;
    r12 = r13;
    r13 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 4) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 3) * 4) +
                          (get_sub_group_local_id() ^ ((1 << 3) - 1));
  shared.m[get_local_id(0) + (4 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_r_idx + (8)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (8)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (16)];
      ulong r1_2 = shared.m[smem_r_idx + (24)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (16)] = r1_1;
      shared.m[smem_r_idx + (24)] = r1_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (128)];
      ulong r0_2 = shared.m[smem_r_idx + (136)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_r_idx + (136)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (144)];
      ulong r1_2 = shared.m[smem_r_idx + (152)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (144)] = r1_1;
      shared.m[smem_r_idx + (152)] = r1_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (256)];
      ulong r0_2 = shared.m[smem_r_idx + (264)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_r_idx + (264)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (272)];
      ulong r1_2 = shared.m[smem_r_idx + (280)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (272)] = r1_1;
      shared.m[smem_r_idx + (280)] = r1_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (384)];
      ulong r0_2 = shared.m[smem_r_idx + (392)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (384)] = r0_1;
      shared.m[smem_r_idx + (392)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (400)];
      ulong r1_2 = shared.m[smem_r_idx + (408)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (400)] = r1_1;
      shared.m[smem_r_idx + (408)] = r1_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (4 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (4 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (4 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (4 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (4 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (4 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (4 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (4 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (4 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (4 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (4 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (4 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (4 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (4 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (4 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (4 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (4 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (4 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_r_idx + (16)];
      ulong r0_4 = shared.m[smem_r_idx + (24)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_r_idx + (16)] = r0_3;
      shared.m[smem_r_idx + (24)] = r0_4;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (128)];
      ulong r0_2 = shared.m[smem_l_idx + (136)];
      ulong r0_3 = shared.m[smem_r_idx + (144)];
      ulong r0_4 = shared.m[smem_r_idx + (152)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_l_idx + (136)] = r0_2;
      shared.m[smem_r_idx + (144)] = r0_3;
      shared.m[smem_r_idx + (152)] = r0_4;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (256)];
      ulong r0_2 = shared.m[smem_l_idx + (264)];
      ulong r0_3 = shared.m[smem_r_idx + (272)];
      ulong r0_4 = shared.m[smem_r_idx + (280)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_l_idx + (264)] = r0_2;
      shared.m[smem_r_idx + (272)] = r0_3;
      shared.m[smem_r_idx + (280)] = r0_4;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (384)];
      ulong r0_2 = shared.m[smem_l_idx + (392)];
      ulong r0_3 = shared.m[smem_r_idx + (400)];
      ulong r0_4 = shared.m[smem_r_idx + (408)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (384)] = r0_1;
      shared.m[smem_l_idx + (392)] = r0_2;
      shared.m[smem_r_idx + (400)] = r0_3;
      shared.m[smem_r_idx + (408)] = r0_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (4 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (4 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (4 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (4 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (4 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (4 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (4 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (4 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (4 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (4 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (4 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (4 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (4 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (4 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (4 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (4 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 8, 1, 1))) void
hs_kernel_bs_3(__global ulong const* const restrict vin,
               __global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[64 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vin[gmem_idx + (1 << 3) * 0];
  ulong r2 = vin[gmem_idx + (1 << 3) * 1];
  ulong r3 = vin[gmem_idx + (1 << 3) * 2];
  ulong r4 = vin[gmem_idx + (1 << 3) * 3];
  ulong r5 = vin[gmem_idx + (1 << 3) * 4];
  ulong r6 = vin[gmem_idx + (1 << 3) * 5];
  ulong r7 = vin[gmem_idx + (1 << 3) * 6];
  ulong r8 = vin[gmem_idx + (1 << 3) * 7];
  ulong r9 = vin[gmem_idx + (1 << 3) * 8];
  ulong r10 = vin[gmem_idx + (1 << 3) * 9];
  ulong r11 = vin[gmem_idx + (1 << 3) * 10];
  ulong r12 = vin[gmem_idx + (1 << 3) * 11];
  ulong r13 = vin[gmem_idx + (1 << 3) * 12];
  ulong r14 = vin[gmem_idx + (1 << 3) * 13];
  ulong r15 = vin[gmem_idx + (1 << 3) * 14];
  ulong r16 = vin[gmem_idx + (1 << 3) * 15];
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r6 >= r11) {
    ulong const t = r6;
    r6 = r11;
    r11 = t;
  };
  if (r7 >= r10) {
    ulong const t = r7;
    r7 = r10;
    r10 = t;
  };
  if (r4 >= r13) {
    ulong const t = r4;
    r4 = r13;
    r13 = t;
  };
  if (r14 >= r15) {
    ulong const t = r14;
    r14 = r15;
    r15 = t;
  };
  if (r8 >= r12) {
    ulong const t = r8;
    r8 = r12;
    r12 = t;
  };
  if (r2 >= r3) {
    ulong const t = r2;
    r2 = r3;
    r3 = t;
  };
  if (r5 >= r9) {
    ulong const t = r5;
    r5 = r9;
    r9 = t;
  };
  if (r2 >= r5) {
    ulong const t = r2;
    r2 = r5;
    r5 = t;
  };
  if (r8 >= r14) {
    ulong const t = r8;
    r8 = r14;
    r14 = t;
  };
  if (r3 >= r9) {
    ulong const t = r3;
    r3 = r9;
    r9 = t;
  };
  if (r12 >= r15) {
    ulong const t = r12;
    r12 = r15;
    r15 = t;
  };
  if (r3 >= r5) {
    ulong const t = r3;
    r3 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r14) {
    ulong const t = r12;
    r12 = r14;
    r14 = t;
  };
  if (r4 >= r9) {
    ulong const t = r4;
    r4 = r9;
    r9 = t;
  };
  if (r8 >= r13) {
    ulong const t = r8;
    r8 = r13;
    r13 = t;
  };
  if (r7 >= r9) {
    ulong const t = r7;
    r7 = r9;
    r9 = t;
  };
  if (r11 >= r13) {
    ulong const t = r11;
    r11 = r13;
    r13 = t;
  };
  if (r4 >= r6) {
    ulong const t = r4;
    r4 = r6;
    r6 = t;
  };
  if (r8 >= r10) {
    ulong const t = r8;
    r8 = r10;
    r10 = t;
  };
  if (r4 >= r5) {
    ulong const t = r4;
    r4 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r8 >= r9) {
    ulong const t = r8;
    r8 = r9;
    r9 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r13) {
    ulong const t = r12;
    r12 = r13;
    r13 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 8) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 3) * 8) +
                          (get_sub_group_local_id() ^ ((1 << 3) - 1));
  shared.m[get_local_id(0) + (8 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_r_idx + (8)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (8)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (16)];
      ulong r1_2 = shared.m[smem_r_idx + (24)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (16)] = r1_1;
      shared.m[smem_r_idx + (24)] = r1_2;
    }
    {
      ulong r2_1 = shared.m[smem_l_idx + (32)];
      ulong r2_2 = shared.m[smem_r_idx + (40)];
      if (r2_1 >= r2_2) {
        ulong const t = r2_1;
        r2_1 = r2_2;
        r2_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r2_1;
      shared.m[smem_r_idx + (40)] = r2_2;
    }
    {
      ulong r3_1 = shared.m[smem_l_idx + (48)];
      ulong r3_2 = shared.m[smem_r_idx + (56)];
      if (r3_1 >= r3_2) {
        ulong const t = r3_1;
        r3_1 = r3_2;
        r3_2 = t;
      };
      shared.m[smem_l_idx + (48)] = r3_1;
      shared.m[smem_r_idx + (56)] = r3_2;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (512)];
      ulong r0_2 = shared.m[smem_r_idx + (520)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (512)] = r0_1;
      shared.m[smem_r_idx + (520)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (528)];
      ulong r1_2 = shared.m[smem_r_idx + (536)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (528)] = r1_1;
      shared.m[smem_r_idx + (536)] = r1_2;
    }
    {
      ulong r2_1 = shared.m[smem_l_idx + (544)];
      ulong r2_2 = shared.m[smem_r_idx + (552)];
      if (r2_1 >= r2_2) {
        ulong const t = r2_1;
        r2_1 = r2_2;
        r2_2 = t;
      };
      shared.m[smem_l_idx + (544)] = r2_1;
      shared.m[smem_r_idx + (552)] = r2_2;
    }
    {
      ulong r3_1 = shared.m[smem_l_idx + (560)];
      ulong r3_2 = shared.m[smem_r_idx + (568)];
      if (r3_1 >= r3_2) {
        ulong const t = r3_1;
        r3_1 = r3_2;
        r3_2 = t;
      };
      shared.m[smem_l_idx + (560)] = r3_1;
      shared.m[smem_r_idx + (568)] = r3_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (8 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (8 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (8 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (8 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (8 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (8 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (8 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (8 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (8 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_r_idx + (16)];
      ulong r0_4 = shared.m[smem_r_idx + (24)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_r_idx + (16)] = r0_3;
      shared.m[smem_r_idx + (24)] = r0_4;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (32)];
      ulong r1_2 = shared.m[smem_l_idx + (40)];
      ulong r1_3 = shared.m[smem_r_idx + (48)];
      ulong r1_4 = shared.m[smem_r_idx + (56)];
      if (r1_2 >= r1_3) {
        ulong const t = r1_2;
        r1_2 = r1_3;
        r1_3 = t;
      };
      if (r1_1 >= r1_4) {
        ulong const t = r1_1;
        r1_1 = r1_4;
        r1_4 = t;
      };
      if (r1_3 >= r1_4) {
        ulong const t = r1_3;
        r1_3 = r1_4;
        r1_4 = t;
      };
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r1_1;
      shared.m[smem_l_idx + (40)] = r1_2;
      shared.m[smem_r_idx + (48)] = r1_3;
      shared.m[smem_r_idx + (56)] = r1_4;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (512)];
      ulong r0_2 = shared.m[smem_l_idx + (520)];
      ulong r0_3 = shared.m[smem_r_idx + (528)];
      ulong r0_4 = shared.m[smem_r_idx + (536)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (512)] = r0_1;
      shared.m[smem_l_idx + (520)] = r0_2;
      shared.m[smem_r_idx + (528)] = r0_3;
      shared.m[smem_r_idx + (536)] = r0_4;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (544)];
      ulong r1_2 = shared.m[smem_l_idx + (552)];
      ulong r1_3 = shared.m[smem_r_idx + (560)];
      ulong r1_4 = shared.m[smem_r_idx + (568)];
      if (r1_2 >= r1_3) {
        ulong const t = r1_2;
        r1_2 = r1_3;
        r1_3 = t;
      };
      if (r1_1 >= r1_4) {
        ulong const t = r1_1;
        r1_1 = r1_4;
        r1_4 = t;
      };
      if (r1_3 >= r1_4) {
        ulong const t = r1_3;
        r1_3 = r1_4;
        r1_4 = t;
      };
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (544)] = r1_1;
      shared.m[smem_l_idx + (552)] = r1_2;
      shared.m[smem_r_idx + (560)] = r1_3;
      shared.m[smem_r_idx + (568)] = r1_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (8 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (8 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (8 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (8 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (8 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (8 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (8 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (8 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (8 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_l_idx + (16)];
      ulong r0_4 = shared.m[smem_l_idx + (24)];
      ulong r0_5 = shared.m[smem_r_idx + (32)];
      ulong r0_6 = shared.m[smem_r_idx + (40)];
      ulong r0_7 = shared.m[smem_r_idx + (48)];
      ulong r0_8 = shared.m[smem_r_idx + (56)];
      if (r0_4 >= r0_5) {
        ulong const t = r0_4;
        r0_4 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_6) {
        ulong const t = r0_3;
        r0_3 = r0_6;
        r0_6 = t;
      };
      if (r0_2 >= r0_7) {
        ulong const t = r0_2;
        r0_2 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_8) {
        ulong const t = r0_1;
        r0_1 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
      shared.m[smem_r_idx + (32)] = r0_5;
      shared.m[smem_r_idx + (40)] = r0_6;
      shared.m[smem_r_idx + (48)] = r0_7;
      shared.m[smem_r_idx + (56)] = r0_8;
    }
    {
      ulong r0_1 = shared.m[smem_l_idx + (512)];
      ulong r0_2 = shared.m[smem_l_idx + (520)];
      ulong r0_3 = shared.m[smem_l_idx + (528)];
      ulong r0_4 = shared.m[smem_l_idx + (536)];
      ulong r0_5 = shared.m[smem_r_idx + (544)];
      ulong r0_6 = shared.m[smem_r_idx + (552)];
      ulong r0_7 = shared.m[smem_r_idx + (560)];
      ulong r0_8 = shared.m[smem_r_idx + (568)];
      if (r0_4 >= r0_5) {
        ulong const t = r0_4;
        r0_4 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_6) {
        ulong const t = r0_3;
        r0_3 = r0_6;
        r0_6 = t;
      };
      if (r0_2 >= r0_7) {
        ulong const t = r0_2;
        r0_2 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_8) {
        ulong const t = r0_1;
        r0_1 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (512)] = r0_1;
      shared.m[smem_l_idx + (520)] = r0_2;
      shared.m[smem_l_idx + (528)] = r0_3;
      shared.m[smem_l_idx + (536)] = r0_4;
      shared.m[smem_r_idx + (544)] = r0_5;
      shared.m[smem_r_idx + (552)] = r0_6;
      shared.m[smem_r_idx + (560)] = r0_7;
      shared.m[smem_r_idx + (568)] = r0_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (8 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (8 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (8 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (8 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (8 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (8 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (8 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (8 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 16, 1, 1))) void
hs_kernel_bs_4(__global ulong const* const restrict vin,
               __global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[128 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vin[gmem_idx + (1 << 3) * 0];
  ulong r2 = vin[gmem_idx + (1 << 3) * 1];
  ulong r3 = vin[gmem_idx + (1 << 3) * 2];
  ulong r4 = vin[gmem_idx + (1 << 3) * 3];
  ulong r5 = vin[gmem_idx + (1 << 3) * 4];
  ulong r6 = vin[gmem_idx + (1 << 3) * 5];
  ulong r7 = vin[gmem_idx + (1 << 3) * 6];
  ulong r8 = vin[gmem_idx + (1 << 3) * 7];
  ulong r9 = vin[gmem_idx + (1 << 3) * 8];
  ulong r10 = vin[gmem_idx + (1 << 3) * 9];
  ulong r11 = vin[gmem_idx + (1 << 3) * 10];
  ulong r12 = vin[gmem_idx + (1 << 3) * 11];
  ulong r13 = vin[gmem_idx + (1 << 3) * 12];
  ulong r14 = vin[gmem_idx + (1 << 3) * 13];
  ulong r15 = vin[gmem_idx + (1 << 3) * 14];
  ulong r16 = vin[gmem_idx + (1 << 3) * 15];
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r6 >= r11) {
    ulong const t = r6;
    r6 = r11;
    r11 = t;
  };
  if (r7 >= r10) {
    ulong const t = r7;
    r7 = r10;
    r10 = t;
  };
  if (r4 >= r13) {
    ulong const t = r4;
    r4 = r13;
    r13 = t;
  };
  if (r14 >= r15) {
    ulong const t = r14;
    r14 = r15;
    r15 = t;
  };
  if (r8 >= r12) {
    ulong const t = r8;
    r8 = r12;
    r12 = t;
  };
  if (r2 >= r3) {
    ulong const t = r2;
    r2 = r3;
    r3 = t;
  };
  if (r5 >= r9) {
    ulong const t = r5;
    r5 = r9;
    r9 = t;
  };
  if (r2 >= r5) {
    ulong const t = r2;
    r2 = r5;
    r5 = t;
  };
  if (r8 >= r14) {
    ulong const t = r8;
    r8 = r14;
    r14 = t;
  };
  if (r3 >= r9) {
    ulong const t = r3;
    r3 = r9;
    r9 = t;
  };
  if (r12 >= r15) {
    ulong const t = r12;
    r12 = r15;
    r15 = t;
  };
  if (r3 >= r5) {
    ulong const t = r3;
    r3 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r14) {
    ulong const t = r12;
    r12 = r14;
    r14 = t;
  };
  if (r4 >= r9) {
    ulong const t = r4;
    r4 = r9;
    r9 = t;
  };
  if (r8 >= r13) {
    ulong const t = r8;
    r8 = r13;
    r13 = t;
  };
  if (r7 >= r9) {
    ulong const t = r7;
    r7 = r9;
    r9 = t;
  };
  if (r11 >= r13) {
    ulong const t = r11;
    r11 = r13;
    r13 = t;
  };
  if (r4 >= r6) {
    ulong const t = r4;
    r4 = r6;
    r6 = t;
  };
  if (r8 >= r10) {
    ulong const t = r8;
    r8 = r10;
    r10 = t;
  };
  if (r4 >= r5) {
    ulong const t = r4;
    r4 = r5;
    r5 = t;
  };
  if (r6 >= r7) {
    ulong const t = r6;
    r6 = r7;
    r7 = t;
  };
  if (r8 >= r9) {
    ulong const t = r8;
    r8 = r9;
    r9 = t;
  };
  if (r10 >= r11) {
    ulong const t = r10;
    r10 = r11;
    r11 = t;
  };
  if (r12 >= r13) {
    ulong const t = r12;
    r12 = r13;
    r13 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r16, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r15, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r14, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r13, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r12, flip_lane_idx);
      r5 = ((r5 <= tb) ^ t_lt) ? tb : r5;
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r11, flip_lane_idx);
      r6 = ((r6 <= tb) ^ t_lt) ? tb : r6;
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r10, flip_lane_idx);
      r7 = ((r7 <= tb) ^ t_lt) ? tb : r7;
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, flip_lane_idx);
      ulong const tb = intel_sub_group_shuffle(r9, flip_lane_idx);
      r8 = ((r8 <= tb) ^ t_lt) ? tb : r8;
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
      r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
      r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
      r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
      r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
      r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
      r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
      r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
    };
    {
      ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
      r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
    };
  }
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 16) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 3) * 16) +
                          (get_sub_group_local_id() ^ ((1 << 3) - 1));
  shared.m[get_local_id(0) + (16 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_r_idx + (8)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (8)] = r0_2;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (16)];
      ulong r1_2 = shared.m[smem_r_idx + (24)];
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (16)] = r1_1;
      shared.m[smem_r_idx + (24)] = r1_2;
    }
    {
      ulong r2_1 = shared.m[smem_l_idx + (32)];
      ulong r2_2 = shared.m[smem_r_idx + (40)];
      if (r2_1 >= r2_2) {
        ulong const t = r2_1;
        r2_1 = r2_2;
        r2_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r2_1;
      shared.m[smem_r_idx + (40)] = r2_2;
    }
    {
      ulong r3_1 = shared.m[smem_l_idx + (48)];
      ulong r3_2 = shared.m[smem_r_idx + (56)];
      if (r3_1 >= r3_2) {
        ulong const t = r3_1;
        r3_1 = r3_2;
        r3_2 = t;
      };
      shared.m[smem_l_idx + (48)] = r3_1;
      shared.m[smem_r_idx + (56)] = r3_2;
    }
    {
      ulong r4_1 = shared.m[smem_l_idx + (64)];
      ulong r4_2 = shared.m[smem_r_idx + (72)];
      if (r4_1 >= r4_2) {
        ulong const t = r4_1;
        r4_1 = r4_2;
        r4_2 = t;
      };
      shared.m[smem_l_idx + (64)] = r4_1;
      shared.m[smem_r_idx + (72)] = r4_2;
    }
    {
      ulong r5_1 = shared.m[smem_l_idx + (80)];
      ulong r5_2 = shared.m[smem_r_idx + (88)];
      if (r5_1 >= r5_2) {
        ulong const t = r5_1;
        r5_1 = r5_2;
        r5_2 = t;
      };
      shared.m[smem_l_idx + (80)] = r5_1;
      shared.m[smem_r_idx + (88)] = r5_2;
    }
    {
      ulong r6_1 = shared.m[smem_l_idx + (96)];
      ulong r6_2 = shared.m[smem_r_idx + (104)];
      if (r6_1 >= r6_2) {
        ulong const t = r6_1;
        r6_1 = r6_2;
        r6_2 = t;
      };
      shared.m[smem_l_idx + (96)] = r6_1;
      shared.m[smem_r_idx + (104)] = r6_2;
    }
    {
      ulong r7_1 = shared.m[smem_l_idx + (112)];
      ulong r7_2 = shared.m[smem_r_idx + (120)];
      if (r7_1 >= r7_2) {
        ulong const t = r7_1;
        r7_1 = r7_2;
        r7_2 = t;
      };
      shared.m[smem_l_idx + (112)] = r7_1;
      shared.m[smem_r_idx + (120)] = r7_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (16 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (16 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (16 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (16 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (16 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (16 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (16 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (16 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_r_idx + (16)];
      ulong r0_4 = shared.m[smem_r_idx + (24)];
      if (r0_2 >= r0_3) {
        ulong const t = r0_2;
        r0_2 = r0_3;
        r0_3 = t;
      };
      if (r0_1 >= r0_4) {
        ulong const t = r0_1;
        r0_1 = r0_4;
        r0_4 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_r_idx + (16)] = r0_3;
      shared.m[smem_r_idx + (24)] = r0_4;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (32)];
      ulong r1_2 = shared.m[smem_l_idx + (40)];
      ulong r1_3 = shared.m[smem_r_idx + (48)];
      ulong r1_4 = shared.m[smem_r_idx + (56)];
      if (r1_2 >= r1_3) {
        ulong const t = r1_2;
        r1_2 = r1_3;
        r1_3 = t;
      };
      if (r1_1 >= r1_4) {
        ulong const t = r1_1;
        r1_1 = r1_4;
        r1_4 = t;
      };
      if (r1_3 >= r1_4) {
        ulong const t = r1_3;
        r1_3 = r1_4;
        r1_4 = t;
      };
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r1_1;
      shared.m[smem_l_idx + (40)] = r1_2;
      shared.m[smem_r_idx + (48)] = r1_3;
      shared.m[smem_r_idx + (56)] = r1_4;
    }
    {
      ulong r2_1 = shared.m[smem_l_idx + (64)];
      ulong r2_2 = shared.m[smem_l_idx + (72)];
      ulong r2_3 = shared.m[smem_r_idx + (80)];
      ulong r2_4 = shared.m[smem_r_idx + (88)];
      if (r2_2 >= r2_3) {
        ulong const t = r2_2;
        r2_2 = r2_3;
        r2_3 = t;
      };
      if (r2_1 >= r2_4) {
        ulong const t = r2_1;
        r2_1 = r2_4;
        r2_4 = t;
      };
      if (r2_3 >= r2_4) {
        ulong const t = r2_3;
        r2_3 = r2_4;
        r2_4 = t;
      };
      if (r2_1 >= r2_2) {
        ulong const t = r2_1;
        r2_1 = r2_2;
        r2_2 = t;
      };
      shared.m[smem_l_idx + (64)] = r2_1;
      shared.m[smem_l_idx + (72)] = r2_2;
      shared.m[smem_r_idx + (80)] = r2_3;
      shared.m[smem_r_idx + (88)] = r2_4;
    }
    {
      ulong r3_1 = shared.m[smem_l_idx + (96)];
      ulong r3_2 = shared.m[smem_l_idx + (104)];
      ulong r3_3 = shared.m[smem_r_idx + (112)];
      ulong r3_4 = shared.m[smem_r_idx + (120)];
      if (r3_2 >= r3_3) {
        ulong const t = r3_2;
        r3_2 = r3_3;
        r3_3 = t;
      };
      if (r3_1 >= r3_4) {
        ulong const t = r3_1;
        r3_1 = r3_4;
        r3_4 = t;
      };
      if (r3_3 >= r3_4) {
        ulong const t = r3_3;
        r3_3 = r3_4;
        r3_4 = t;
      };
      if (r3_1 >= r3_2) {
        ulong const t = r3_1;
        r3_1 = r3_2;
        r3_2 = t;
      };
      shared.m[smem_l_idx + (96)] = r3_1;
      shared.m[smem_l_idx + (104)] = r3_2;
      shared.m[smem_r_idx + (112)] = r3_3;
      shared.m[smem_r_idx + (120)] = r3_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (16 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (16 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (16 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (16 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (16 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (16 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (16 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (16 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_l_idx + (16)];
      ulong r0_4 = shared.m[smem_l_idx + (24)];
      ulong r0_5 = shared.m[smem_r_idx + (32)];
      ulong r0_6 = shared.m[smem_r_idx + (40)];
      ulong r0_7 = shared.m[smem_r_idx + (48)];
      ulong r0_8 = shared.m[smem_r_idx + (56)];
      if (r0_4 >= r0_5) {
        ulong const t = r0_4;
        r0_4 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_6) {
        ulong const t = r0_3;
        r0_3 = r0_6;
        r0_6 = t;
      };
      if (r0_2 >= r0_7) {
        ulong const t = r0_2;
        r0_2 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_8) {
        ulong const t = r0_1;
        r0_1 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
      shared.m[smem_r_idx + (32)] = r0_5;
      shared.m[smem_r_idx + (40)] = r0_6;
      shared.m[smem_r_idx + (48)] = r0_7;
      shared.m[smem_r_idx + (56)] = r0_8;
    }
    {
      ulong r1_1 = shared.m[smem_l_idx + (64)];
      ulong r1_2 = shared.m[smem_l_idx + (72)];
      ulong r1_3 = shared.m[smem_l_idx + (80)];
      ulong r1_4 = shared.m[smem_l_idx + (88)];
      ulong r1_5 = shared.m[smem_r_idx + (96)];
      ulong r1_6 = shared.m[smem_r_idx + (104)];
      ulong r1_7 = shared.m[smem_r_idx + (112)];
      ulong r1_8 = shared.m[smem_r_idx + (120)];
      if (r1_4 >= r1_5) {
        ulong const t = r1_4;
        r1_4 = r1_5;
        r1_5 = t;
      };
      if (r1_3 >= r1_6) {
        ulong const t = r1_3;
        r1_3 = r1_6;
        r1_6 = t;
      };
      if (r1_2 >= r1_7) {
        ulong const t = r1_2;
        r1_2 = r1_7;
        r1_7 = t;
      };
      if (r1_1 >= r1_8) {
        ulong const t = r1_1;
        r1_1 = r1_8;
        r1_8 = t;
      };
      if (r1_5 >= r1_7) {
        ulong const t = r1_5;
        r1_5 = r1_7;
        r1_7 = t;
      };
      if (r1_6 >= r1_8) {
        ulong const t = r1_6;
        r1_6 = r1_8;
        r1_8 = t;
      };
      if (r1_5 >= r1_6) {
        ulong const t = r1_5;
        r1_5 = r1_6;
        r1_6 = t;
      };
      if (r1_7 >= r1_8) {
        ulong const t = r1_7;
        r1_7 = r1_8;
        r1_8 = t;
      };
      if (r1_1 >= r1_3) {
        ulong const t = r1_1;
        r1_1 = r1_3;
        r1_3 = t;
      };
      if (r1_2 >= r1_4) {
        ulong const t = r1_2;
        r1_2 = r1_4;
        r1_4 = t;
      };
      if (r1_1 >= r1_2) {
        ulong const t = r1_1;
        r1_1 = r1_2;
        r1_2 = t;
      };
      if (r1_3 >= r1_4) {
        ulong const t = r1_3;
        r1_3 = r1_4;
        r1_4 = t;
      };
      shared.m[smem_l_idx + (64)] = r1_1;
      shared.m[smem_l_idx + (72)] = r1_2;
      shared.m[smem_l_idx + (80)] = r1_3;
      shared.m[smem_l_idx + (88)] = r1_4;
      shared.m[smem_r_idx + (96)] = r1_5;
      shared.m[smem_r_idx + (104)] = r1_6;
      shared.m[smem_r_idx + (112)] = r1_7;
      shared.m[smem_r_idx + (120)] = r1_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (16 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (16 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (16 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (16 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (16 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (16 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (16 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (16 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 3) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 1)] = r16;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 3)] = r15;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 5)] = r14;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 7)] = r13;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 8)] = r5;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 9)] = r12;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 10)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 11)] = r11;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 12)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 13)] = r10;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 14)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 3) * 15)] = r9;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      ulong r0_1 = shared.m[smem_l_idx + (0)];
      ulong r0_2 = shared.m[smem_l_idx + (8)];
      ulong r0_3 = shared.m[smem_l_idx + (16)];
      ulong r0_4 = shared.m[smem_l_idx + (24)];
      ulong r0_5 = shared.m[smem_l_idx + (32)];
      ulong r0_6 = shared.m[smem_l_idx + (40)];
      ulong r0_7 = shared.m[smem_l_idx + (48)];
      ulong r0_8 = shared.m[smem_l_idx + (56)];
      ulong r0_9 = shared.m[smem_r_idx + (64)];
      ulong r0_10 = shared.m[smem_r_idx + (72)];
      ulong r0_11 = shared.m[smem_r_idx + (80)];
      ulong r0_12 = shared.m[smem_r_idx + (88)];
      ulong r0_13 = shared.m[smem_r_idx + (96)];
      ulong r0_14 = shared.m[smem_r_idx + (104)];
      ulong r0_15 = shared.m[smem_r_idx + (112)];
      ulong r0_16 = shared.m[smem_r_idx + (120)];
      if (r0_8 >= r0_9) {
        ulong const t = r0_8;
        r0_8 = r0_9;
        r0_9 = t;
      };
      if (r0_7 >= r0_10) {
        ulong const t = r0_7;
        r0_7 = r0_10;
        r0_10 = t;
      };
      if (r0_6 >= r0_11) {
        ulong const t = r0_6;
        r0_6 = r0_11;
        r0_11 = t;
      };
      if (r0_5 >= r0_12) {
        ulong const t = r0_5;
        r0_5 = r0_12;
        r0_12 = t;
      };
      if (r0_4 >= r0_13) {
        ulong const t = r0_4;
        r0_4 = r0_13;
        r0_13 = t;
      };
      if (r0_3 >= r0_14) {
        ulong const t = r0_3;
        r0_3 = r0_14;
        r0_14 = t;
      };
      if (r0_2 >= r0_15) {
        ulong const t = r0_2;
        r0_2 = r0_15;
        r0_15 = t;
      };
      if (r0_1 >= r0_16) {
        ulong const t = r0_1;
        r0_1 = r0_16;
        r0_16 = t;
      };
      if (r0_9 >= r0_13) {
        ulong const t = r0_9;
        r0_9 = r0_13;
        r0_13 = t;
      };
      if (r0_11 >= r0_15) {
        ulong const t = r0_11;
        r0_11 = r0_15;
        r0_15 = t;
      };
      if (r0_9 >= r0_11) {
        ulong const t = r0_9;
        r0_9 = r0_11;
        r0_11 = t;
      };
      if (r0_13 >= r0_15) {
        ulong const t = r0_13;
        r0_13 = r0_15;
        r0_15 = t;
      };
      if (r0_10 >= r0_14) {
        ulong const t = r0_10;
        r0_10 = r0_14;
        r0_14 = t;
      };
      if (r0_12 >= r0_16) {
        ulong const t = r0_12;
        r0_12 = r0_16;
        r0_16 = t;
      };
      if (r0_10 >= r0_12) {
        ulong const t = r0_10;
        r0_10 = r0_12;
        r0_12 = t;
      };
      if (r0_14 >= r0_16) {
        ulong const t = r0_14;
        r0_14 = r0_16;
        r0_16 = t;
      };
      if (r0_9 >= r0_10) {
        ulong const t = r0_9;
        r0_9 = r0_10;
        r0_10 = t;
      };
      if (r0_11 >= r0_12) {
        ulong const t = r0_11;
        r0_11 = r0_12;
        r0_12 = t;
      };
      if (r0_13 >= r0_14) {
        ulong const t = r0_13;
        r0_13 = r0_14;
        r0_14 = t;
      };
      if (r0_15 >= r0_16) {
        ulong const t = r0_15;
        r0_15 = r0_16;
        r0_16 = t;
      };
      if (r0_1 >= r0_5) {
        ulong const t = r0_1;
        r0_1 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_7) {
        ulong const t = r0_3;
        r0_3 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_2 >= r0_6) {
        ulong const t = r0_2;
        r0_2 = r0_6;
        r0_6 = t;
      };
      if (r0_4 >= r0_8) {
        ulong const t = r0_4;
        r0_4 = r0_8;
        r0_8 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
      shared.m[smem_l_idx + (32)] = r0_5;
      shared.m[smem_l_idx + (40)] = r0_6;
      shared.m[smem_l_idx + (48)] = r0_7;
      shared.m[smem_l_idx + (56)] = r0_8;
      shared.m[smem_r_idx + (64)] = r0_9;
      shared.m[smem_r_idx + (72)] = r0_10;
      shared.m[smem_r_idx + (80)] = r0_11;
      shared.m[smem_r_idx + (88)] = r0_12;
      shared.m[smem_r_idx + (96)] = r0_13;
      shared.m[smem_r_idx + (104)] = r0_14;
      shared.m[smem_r_idx + (112)] = r0_15;
      shared.m[smem_r_idx + (120)] = r0_16;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 3) * 0)];
  r16 = shared.m[get_local_id(0) + (16 * (1 << 3) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 3) * 2)];
  r15 = shared.m[get_local_id(0) + (16 * (1 << 3) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 3) * 4)];
  r14 = shared.m[get_local_id(0) + (16 * (1 << 3) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 3) * 6)];
  r13 = shared.m[get_local_id(0) + (16 * (1 << 3) * 7)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 3) * 8)];
  r12 = shared.m[get_local_id(0) + (16 * (1 << 3) * 9)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 3) * 10)];
  r11 = shared.m[get_local_id(0) + (16 * (1 << 3) * 11)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 3) * 12)];
  r10 = shared.m[get_local_id(0) + (16 * (1 << 3) * 13)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 3) * 14)];
  r9 = shared.m[get_local_id(0) + (16 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 1, 1, 1))) void
hs_kernel_bc_0(__global ulong* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vout[gmem_idx + (1 << 3) * 0];
  ulong r2 = vout[gmem_idx + (1 << 3) * 1];
  ulong r3 = vout[gmem_idx + (1 << 3) * 2];
  ulong r4 = vout[gmem_idx + (1 << 3) * 3];
  ulong r5 = vout[gmem_idx + (1 << 3) * 4];
  ulong r6 = vout[gmem_idx + (1 << 3) * 5];
  ulong r7 = vout[gmem_idx + (1 << 3) * 6];
  ulong r8 = vout[gmem_idx + (1 << 3) * 7];
  ulong r9 = vout[gmem_idx + (1 << 3) * 8];
  ulong r10 = vout[gmem_idx + (1 << 3) * 9];
  ulong r11 = vout[gmem_idx + (1 << 3) * 10];
  ulong r12 = vout[gmem_idx + (1 << 3) * 11];
  ulong r13 = vout[gmem_idx + (1 << 3) * 12];
  ulong r14 = vout[gmem_idx + (1 << 3) * 13];
  ulong r15 = vout[gmem_idx + (1 << 3) * 14];
  ulong r16 = vout[gmem_idx + (1 << 3) * 15];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 2, 1, 1))) void
hs_kernel_bc_1(__global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[16 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 3) * 2 - 1)) * 16 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 2) + get_sub_group_local_id();
  {
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 0)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 16)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 2)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 18)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (32)] = r0_1;
      shared.m[smem_l_idx + (40)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 4)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 20)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (64)] = r0_1;
      shared.m[smem_l_idx + (72)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 6)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 22)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (96)] = r0_1;
      shared.m[smem_l_idx + (104)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 8)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 24)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_l_idx + (136)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 10)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 26)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (160)] = r0_1;
      shared.m[smem_l_idx + (168)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 12)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 28)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (192)] = r0_1;
      shared.m[smem_l_idx + (200)] = r0_2;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 14)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 30)];
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      shared.m[smem_l_idx + (224)] = r0_1;
      shared.m[smem_l_idx + (232)] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  ulong r1 = shared.m[get_local_id(0) + (2 * (1 << 3) * 0)];
  ulong r2 = shared.m[get_local_id(0) + (2 * (1 << 3) * 1)];
  ulong r3 = shared.m[get_local_id(0) + (2 * (1 << 3) * 2)];
  ulong r4 = shared.m[get_local_id(0) + (2 * (1 << 3) * 3)];
  ulong r5 = shared.m[get_local_id(0) + (2 * (1 << 3) * 4)];
  ulong r6 = shared.m[get_local_id(0) + (2 * (1 << 3) * 5)];
  ulong r7 = shared.m[get_local_id(0) + (2 * (1 << 3) * 6)];
  ulong r8 = shared.m[get_local_id(0) + (2 * (1 << 3) * 7)];
  ulong r9 = shared.m[get_local_id(0) + (2 * (1 << 3) * 8)];
  ulong r10 = shared.m[get_local_id(0) + (2 * (1 << 3) * 9)];
  ulong r11 = shared.m[get_local_id(0) + (2 * (1 << 3) * 10)];
  ulong r12 = shared.m[get_local_id(0) + (2 * (1 << 3) * 11)];
  ulong r13 = shared.m[get_local_id(0) + (2 * (1 << 3) * 12)];
  ulong r14 = shared.m[get_local_id(0) + (2 * (1 << 3) * 13)];
  ulong r15 = shared.m[get_local_id(0) + (2 * (1 << 3) * 14)];
  ulong r16 = shared.m[get_local_id(0) + (2 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 4, 1, 1))) void
hs_kernel_bc_2(__global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[32 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 3) * 4 - 1)) * 16 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 4) + get_sub_group_local_id();
  {
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 0)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 16)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 32)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 48)];
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 4)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 20)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 36)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 52)];
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_l_idx + (136)] = r0_2;
      shared.m[smem_l_idx + (144)] = r0_3;
      shared.m[smem_l_idx + (152)] = r0_4;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 8)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 24)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 40)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 56)];
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_l_idx + (264)] = r0_2;
      shared.m[smem_l_idx + (272)] = r0_3;
      shared.m[smem_l_idx + (280)] = r0_4;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 12)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 28)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 44)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 60)];
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      shared.m[smem_l_idx + (384)] = r0_1;
      shared.m[smem_l_idx + (392)] = r0_2;
      shared.m[smem_l_idx + (400)] = r0_3;
      shared.m[smem_l_idx + (408)] = r0_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  ulong r1 = shared.m[get_local_id(0) + (4 * (1 << 3) * 0)];
  ulong r2 = shared.m[get_local_id(0) + (4 * (1 << 3) * 1)];
  ulong r3 = shared.m[get_local_id(0) + (4 * (1 << 3) * 2)];
  ulong r4 = shared.m[get_local_id(0) + (4 * (1 << 3) * 3)];
  ulong r5 = shared.m[get_local_id(0) + (4 * (1 << 3) * 4)];
  ulong r6 = shared.m[get_local_id(0) + (4 * (1 << 3) * 5)];
  ulong r7 = shared.m[get_local_id(0) + (4 * (1 << 3) * 6)];
  ulong r8 = shared.m[get_local_id(0) + (4 * (1 << 3) * 7)];
  ulong r9 = shared.m[get_local_id(0) + (4 * (1 << 3) * 8)];
  ulong r10 = shared.m[get_local_id(0) + (4 * (1 << 3) * 9)];
  ulong r11 = shared.m[get_local_id(0) + (4 * (1 << 3) * 10)];
  ulong r12 = shared.m[get_local_id(0) + (4 * (1 << 3) * 11)];
  ulong r13 = shared.m[get_local_id(0) + (4 * (1 << 3) * 12)];
  ulong r14 = shared.m[get_local_id(0) + (4 * (1 << 3) * 13)];
  ulong r15 = shared.m[get_local_id(0) + (4 * (1 << 3) * 14)];
  ulong r16 = shared.m[get_local_id(0) + (4 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 8, 1, 1))) void
hs_kernel_bc_3(__global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[64 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 3) * 8 - 1)) * 16 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 8) + get_sub_group_local_id();
  {
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 0)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 16)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 32)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 48)];
      ulong r0_5 = vout[gmem_l_idx + ((1 << 3) * 64)];
      ulong r0_6 = vout[gmem_l_idx + ((1 << 3) * 80)];
      ulong r0_7 = vout[gmem_l_idx + ((1 << 3) * 96)];
      ulong r0_8 = vout[gmem_l_idx + ((1 << 3) * 112)];
      if (r0_1 >= r0_5) {
        ulong const t = r0_1;
        r0_1 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_7) {
        ulong const t = r0_3;
        r0_3 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_2 >= r0_6) {
        ulong const t = r0_2;
        r0_2 = r0_6;
        r0_6 = t;
      };
      if (r0_4 >= r0_8) {
        ulong const t = r0_4;
        r0_4 = r0_8;
        r0_8 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
      shared.m[smem_l_idx + (32)] = r0_5;
      shared.m[smem_l_idx + (40)] = r0_6;
      shared.m[smem_l_idx + (48)] = r0_7;
      shared.m[smem_l_idx + (56)] = r0_8;
    }
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 8)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 24)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 40)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 56)];
      ulong r0_5 = vout[gmem_l_idx + ((1 << 3) * 72)];
      ulong r0_6 = vout[gmem_l_idx + ((1 << 3) * 88)];
      ulong r0_7 = vout[gmem_l_idx + ((1 << 3) * 104)];
      ulong r0_8 = vout[gmem_l_idx + ((1 << 3) * 120)];
      if (r0_1 >= r0_5) {
        ulong const t = r0_1;
        r0_1 = r0_5;
        r0_5 = t;
      };
      if (r0_3 >= r0_7) {
        ulong const t = r0_3;
        r0_3 = r0_7;
        r0_7 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_2 >= r0_6) {
        ulong const t = r0_2;
        r0_2 = r0_6;
        r0_6 = t;
      };
      if (r0_4 >= r0_8) {
        ulong const t = r0_4;
        r0_4 = r0_8;
        r0_8 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      shared.m[smem_l_idx + (512)] = r0_1;
      shared.m[smem_l_idx + (520)] = r0_2;
      shared.m[smem_l_idx + (528)] = r0_3;
      shared.m[smem_l_idx + (536)] = r0_4;
      shared.m[smem_l_idx + (544)] = r0_5;
      shared.m[smem_l_idx + (552)] = r0_6;
      shared.m[smem_l_idx + (560)] = r0_7;
      shared.m[smem_l_idx + (568)] = r0_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  ulong r1 = shared.m[get_local_id(0) + (8 * (1 << 3) * 0)];
  ulong r2 = shared.m[get_local_id(0) + (8 * (1 << 3) * 1)];
  ulong r3 = shared.m[get_local_id(0) + (8 * (1 << 3) * 2)];
  ulong r4 = shared.m[get_local_id(0) + (8 * (1 << 3) * 3)];
  ulong r5 = shared.m[get_local_id(0) + (8 * (1 << 3) * 4)];
  ulong r6 = shared.m[get_local_id(0) + (8 * (1 << 3) * 5)];
  ulong r7 = shared.m[get_local_id(0) + (8 * (1 << 3) * 6)];
  ulong r8 = shared.m[get_local_id(0) + (8 * (1 << 3) * 7)];
  ulong r9 = shared.m[get_local_id(0) + (8 * (1 << 3) * 8)];
  ulong r10 = shared.m[get_local_id(0) + (8 * (1 << 3) * 9)];
  ulong r11 = shared.m[get_local_id(0) + (8 * (1 << 3) * 10)];
  ulong r12 = shared.m[get_local_id(0) + (8 * (1 << 3) * 11)];
  ulong r13 = shared.m[get_local_id(0) + (8 * (1 << 3) * 12)];
  ulong r14 = shared.m[get_local_id(0) + (8 * (1 << 3) * 13)];
  ulong r15 = shared.m[get_local_id(0) + (8 * (1 << 3) * 14)];
  ulong r16 = shared.m[get_local_id(0) + (8 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3))))
__attribute__((reqd_work_group_size((1 << 3) * 16, 1, 1))) void
hs_kernel_bc_4(__global ulong* const restrict vout)
{
  __local struct
  {
    ulong m[128 * 16];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 3) * 16 - 1)) * 16 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 3) * 16) + get_sub_group_local_id();
  {
    {
      ulong r0_1 = vout[gmem_l_idx + ((1 << 3) * 0)];
      ulong r0_2 = vout[gmem_l_idx + ((1 << 3) * 16)];
      ulong r0_3 = vout[gmem_l_idx + ((1 << 3) * 32)];
      ulong r0_4 = vout[gmem_l_idx + ((1 << 3) * 48)];
      ulong r0_5 = vout[gmem_l_idx + ((1 << 3) * 64)];
      ulong r0_6 = vout[gmem_l_idx + ((1 << 3) * 80)];
      ulong r0_7 = vout[gmem_l_idx + ((1 << 3) * 96)];
      ulong r0_8 = vout[gmem_l_idx + ((1 << 3) * 112)];
      ulong r0_9 = vout[gmem_l_idx + ((1 << 3) * 128)];
      ulong r0_10 = vout[gmem_l_idx + ((1 << 3) * 144)];
      ulong r0_11 = vout[gmem_l_idx + ((1 << 3) * 160)];
      ulong r0_12 = vout[gmem_l_idx + ((1 << 3) * 176)];
      ulong r0_13 = vout[gmem_l_idx + ((1 << 3) * 192)];
      ulong r0_14 = vout[gmem_l_idx + ((1 << 3) * 208)];
      ulong r0_15 = vout[gmem_l_idx + ((1 << 3) * 224)];
      ulong r0_16 = vout[gmem_l_idx + ((1 << 3) * 240)];
      if (r0_1 >= r0_9) {
        ulong const t = r0_1;
        r0_1 = r0_9;
        r0_9 = t;
      };
      if (r0_5 >= r0_13) {
        ulong const t = r0_5;
        r0_5 = r0_13;
        r0_13 = t;
      };
      if (r0_1 >= r0_5) {
        ulong const t = r0_1;
        r0_1 = r0_5;
        r0_5 = t;
      };
      if (r0_9 >= r0_13) {
        ulong const t = r0_9;
        r0_9 = r0_13;
        r0_13 = t;
      };
      if (r0_3 >= r0_11) {
        ulong const t = r0_3;
        r0_3 = r0_11;
        r0_11 = t;
      };
      if (r0_7 >= r0_15) {
        ulong const t = r0_7;
        r0_7 = r0_15;
        r0_15 = t;
      };
      if (r0_3 >= r0_7) {
        ulong const t = r0_3;
        r0_3 = r0_7;
        r0_7 = t;
      };
      if (r0_11 >= r0_15) {
        ulong const t = r0_11;
        r0_11 = r0_15;
        r0_15 = t;
      };
      if (r0_1 >= r0_3) {
        ulong const t = r0_1;
        r0_1 = r0_3;
        r0_3 = t;
      };
      if (r0_5 >= r0_7) {
        ulong const t = r0_5;
        r0_5 = r0_7;
        r0_7 = t;
      };
      if (r0_9 >= r0_11) {
        ulong const t = r0_9;
        r0_9 = r0_11;
        r0_11 = t;
      };
      if (r0_13 >= r0_15) {
        ulong const t = r0_13;
        r0_13 = r0_15;
        r0_15 = t;
      };
      if (r0_2 >= r0_10) {
        ulong const t = r0_2;
        r0_2 = r0_10;
        r0_10 = t;
      };
      if (r0_6 >= r0_14) {
        ulong const t = r0_6;
        r0_6 = r0_14;
        r0_14 = t;
      };
      if (r0_2 >= r0_6) {
        ulong const t = r0_2;
        r0_2 = r0_6;
        r0_6 = t;
      };
      if (r0_10 >= r0_14) {
        ulong const t = r0_10;
        r0_10 = r0_14;
        r0_14 = t;
      };
      if (r0_4 >= r0_12) {
        ulong const t = r0_4;
        r0_4 = r0_12;
        r0_12 = t;
      };
      if (r0_8 >= r0_16) {
        ulong const t = r0_8;
        r0_8 = r0_16;
        r0_16 = t;
      };
      if (r0_4 >= r0_8) {
        ulong const t = r0_4;
        r0_4 = r0_8;
        r0_8 = t;
      };
      if (r0_12 >= r0_16) {
        ulong const t = r0_12;
        r0_12 = r0_16;
        r0_16 = t;
      };
      if (r0_2 >= r0_4) {
        ulong const t = r0_2;
        r0_2 = r0_4;
        r0_4 = t;
      };
      if (r0_6 >= r0_8) {
        ulong const t = r0_6;
        r0_6 = r0_8;
        r0_8 = t;
      };
      if (r0_10 >= r0_12) {
        ulong const t = r0_10;
        r0_10 = r0_12;
        r0_12 = t;
      };
      if (r0_14 >= r0_16) {
        ulong const t = r0_14;
        r0_14 = r0_16;
        r0_16 = t;
      };
      if (r0_1 >= r0_2) {
        ulong const t = r0_1;
        r0_1 = r0_2;
        r0_2 = t;
      };
      if (r0_3 >= r0_4) {
        ulong const t = r0_3;
        r0_3 = r0_4;
        r0_4 = t;
      };
      if (r0_5 >= r0_6) {
        ulong const t = r0_5;
        r0_5 = r0_6;
        r0_6 = t;
      };
      if (r0_7 >= r0_8) {
        ulong const t = r0_7;
        r0_7 = r0_8;
        r0_8 = t;
      };
      if (r0_9 >= r0_10) {
        ulong const t = r0_9;
        r0_9 = r0_10;
        r0_10 = t;
      };
      if (r0_11 >= r0_12) {
        ulong const t = r0_11;
        r0_11 = r0_12;
        r0_12 = t;
      };
      if (r0_13 >= r0_14) {
        ulong const t = r0_13;
        r0_13 = r0_14;
        r0_14 = t;
      };
      if (r0_15 >= r0_16) {
        ulong const t = r0_15;
        r0_15 = r0_16;
        r0_16 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (8)] = r0_2;
      shared.m[smem_l_idx + (16)] = r0_3;
      shared.m[smem_l_idx + (24)] = r0_4;
      shared.m[smem_l_idx + (32)] = r0_5;
      shared.m[smem_l_idx + (40)] = r0_6;
      shared.m[smem_l_idx + (48)] = r0_7;
      shared.m[smem_l_idx + (56)] = r0_8;
      shared.m[smem_l_idx + (64)] = r0_9;
      shared.m[smem_l_idx + (72)] = r0_10;
      shared.m[smem_l_idx + (80)] = r0_11;
      shared.m[smem_l_idx + (88)] = r0_12;
      shared.m[smem_l_idx + (96)] = r0_13;
      shared.m[smem_l_idx + (104)] = r0_14;
      shared.m[smem_l_idx + (112)] = r0_15;
      shared.m[smem_l_idx + (120)] = r0_16;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  ulong r1 = shared.m[get_local_id(0) + (16 * (1 << 3) * 0)];
  ulong r2 = shared.m[get_local_id(0) + (16 * (1 << 3) * 1)];
  ulong r3 = shared.m[get_local_id(0) + (16 * (1 << 3) * 2)];
  ulong r4 = shared.m[get_local_id(0) + (16 * (1 << 3) * 3)];
  ulong r5 = shared.m[get_local_id(0) + (16 * (1 << 3) * 4)];
  ulong r6 = shared.m[get_local_id(0) + (16 * (1 << 3) * 5)];
  ulong r7 = shared.m[get_local_id(0) + (16 * (1 << 3) * 6)];
  ulong r8 = shared.m[get_local_id(0) + (16 * (1 << 3) * 7)];
  ulong r9 = shared.m[get_local_id(0) + (16 * (1 << 3) * 8)];
  ulong r10 = shared.m[get_local_id(0) + (16 * (1 << 3) * 9)];
  ulong r11 = shared.m[get_local_id(0) + (16 * (1 << 3) * 10)];
  ulong r12 = shared.m[get_local_id(0) + (16 * (1 << 3) * 11)];
  ulong r13 = shared.m[get_local_id(0) + (16 * (1 << 3) * 12)];
  ulong r14 = shared.m[get_local_id(0) + (16 * (1 << 3) * 13)];
  ulong r15 = shared.m[get_local_id(0) + (16 * (1 << 3) * 14)];
  ulong r16 = shared.m[get_local_id(0) + (16 * (1 << 3) * 15)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        ulong const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r9, half_lane_idx);
        r9 = ((r9 <= ta) ^ t_lt) ? ta : r9;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r10, half_lane_idx);
        r10 = ((r10 <= ta) ^ t_lt) ? ta : r10;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r11, half_lane_idx);
        r11 = ((r11 <= ta) ^ t_lt) ? ta : r11;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r12, half_lane_idx);
        r12 = ((r12 <= ta) ^ t_lt) ? ta : r12;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r13, half_lane_idx);
        r13 = ((r13 <= ta) ^ t_lt) ? ta : r13;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r14, half_lane_idx);
        r14 = ((r14 <= ta) ^ t_lt) ? ta : r14;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r15, half_lane_idx);
        r15 = ((r15 <= ta) ^ t_lt) ? ta : r15;
      };
      {
        ulong const ta = intel_sub_group_shuffle(r16, half_lane_idx);
        r16 = ((r16 <= ta) ^ t_lt) ? ta : r16;
      };
    }
    if (r1 >= r9) {
      ulong const t = r1;
      r1 = r9;
      r9 = t;
    };
    if (r5 >= r13) {
      ulong const t = r5;
      r5 = r13;
      r13 = t;
    };
    if (r1 >= r5) {
      ulong const t = r1;
      r1 = r5;
      r5 = t;
    };
    if (r9 >= r13) {
      ulong const t = r9;
      r9 = r13;
      r13 = t;
    };
    if (r3 >= r11) {
      ulong const t = r3;
      r3 = r11;
      r11 = t;
    };
    if (r7 >= r15) {
      ulong const t = r7;
      r7 = r15;
      r15 = t;
    };
    if (r3 >= r7) {
      ulong const t = r3;
      r3 = r7;
      r7 = t;
    };
    if (r11 >= r15) {
      ulong const t = r11;
      r11 = r15;
      r15 = t;
    };
    if (r1 >= r3) {
      ulong const t = r1;
      r1 = r3;
      r3 = t;
    };
    if (r5 >= r7) {
      ulong const t = r5;
      r5 = r7;
      r7 = t;
    };
    if (r9 >= r11) {
      ulong const t = r9;
      r9 = r11;
      r11 = t;
    };
    if (r13 >= r15) {
      ulong const t = r13;
      r13 = r15;
      r15 = t;
    };
    if (r2 >= r10) {
      ulong const t = r2;
      r2 = r10;
      r10 = t;
    };
    if (r6 >= r14) {
      ulong const t = r6;
      r6 = r14;
      r14 = t;
    };
    if (r2 >= r6) {
      ulong const t = r2;
      r2 = r6;
      r6 = t;
    };
    if (r10 >= r14) {
      ulong const t = r10;
      r10 = r14;
      r14 = t;
    };
    if (r4 >= r12) {
      ulong const t = r4;
      r4 = r12;
      r12 = t;
    };
    if (r8 >= r16) {
      ulong const t = r8;
      r8 = r16;
      r16 = t;
    };
    if (r4 >= r8) {
      ulong const t = r4;
      r4 = r8;
      r8 = t;
    };
    if (r12 >= r16) {
      ulong const t = r12;
      r12 = r16;
      r16 = t;
    };
    if (r2 >= r4) {
      ulong const t = r2;
      r2 = r4;
      r4 = t;
    };
    if (r6 >= r8) {
      ulong const t = r6;
      r6 = r8;
      r8 = t;
    };
    if (r10 >= r12) {
      ulong const t = r10;
      r10 = r12;
      r12 = t;
    };
    if (r14 >= r16) {
      ulong const t = r14;
      r14 = r16;
      r16 = t;
    };
    if (r1 >= r2) {
      ulong const t = r1;
      r1 = r2;
      r2 = t;
    };
    if (r3 >= r4) {
      ulong const t = r3;
      r3 = r4;
      r4 = t;
    };
    if (r5 >= r6) {
      ulong const t = r5;
      r5 = r6;
      r6 = t;
    };
    if (r7 >= r8) {
      ulong const t = r7;
      r7 = r8;
      r8 = t;
    };
    if (r9 >= r10) {
      ulong const t = r9;
      r9 = r10;
      r10 = t;
    };
    if (r11 >= r12) {
      ulong const t = r11;
      r11 = r12;
      r12 = t;
    };
    if (r13 >= r14) {
      ulong const t = r13;
      r13 = r14;
      r14 = t;
    };
    if (r15 >= r16) {
      ulong const t = r15;
      r15 = r16;
      r16 = t;
    };
  }
  vout[gmem_idx + (1 << 3) * 0] = r1;
  vout[gmem_idx + (1 << 3) * 1] = r2;
  vout[gmem_idx + (1 << 3) * 2] = r3;
  vout[gmem_idx + (1 << 3) * 3] = r4;
  vout[gmem_idx + (1 << 3) * 4] = r5;
  vout[gmem_idx + (1 << 3) * 5] = r6;
  vout[gmem_idx + (1 << 3) * 6] = r7;
  vout[gmem_idx + (1 << 3) * 7] = r8;
  vout[gmem_idx + (1 << 3) * 8] = r9;
  vout[gmem_idx + (1 << 3) * 9] = r10;
  vout[gmem_idx + (1 << 3) * 10] = r11;
  vout[gmem_idx + (1 << 3) * 11] = r12;
  vout[gmem_idx + (1 << 3) * 12] = r13;
  vout[gmem_idx + (1 << 3) * 13] = r14;
  vout[gmem_idx + (1 << 3) * 14] = r15;
  vout[gmem_idx + (1 << 3) * 15] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_fm_1_0(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (16 + 1) - span_off - 1;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_r + span_stride * 0];
  if (r16 >= r17) {
    ulong const t = r16;
    r16 = r17;
    r17 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_r + span_stride * 0] = r17;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_fm_1_1(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (16 + 1) - span_off - 1;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_r + span_stride * 0];
  ulong r18 = vout[span_r + span_stride * 1];
  if (r16 >= r17) {
    ulong const t = r16;
    r16 = r17;
    r17 = t;
  };
  if (r15 >= r18) {
    ulong const t = r15;
    r15 = r18;
    r18 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r17 >= r18) {
    ulong const t = r17;
    r17 = r18;
    r18 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_r + span_stride * 0] = r17;
  vout[span_r + span_stride * 1] = r18;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_fm_1_2(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (16 + 1) - span_off - 1;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_r + span_stride * 0];
  ulong r18 = vout[span_r + span_stride * 1];
  ulong r19 = vout[span_r + span_stride * 2];
  ulong r20 = vout[span_r + span_stride * 3];
  if (r16 >= r17) {
    ulong const t = r16;
    r16 = r17;
    r17 = t;
  };
  if (r15 >= r18) {
    ulong const t = r15;
    r15 = r18;
    r18 = t;
  };
  if (r14 >= r19) {
    ulong const t = r14;
    r14 = r19;
    r19 = t;
  };
  if (r13 >= r20) {
    ulong const t = r13;
    r13 = r20;
    r20 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r17 >= r19) {
    ulong const t = r17;
    r17 = r19;
    r19 = t;
  };
  if (r18 >= r20) {
    ulong const t = r18;
    r18 = r20;
    r20 = t;
  };
  if (r17 >= r18) {
    ulong const t = r17;
    r17 = r18;
    r18 = t;
  };
  if (r19 >= r20) {
    ulong const t = r19;
    r19 = r20;
    r20 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_r + span_stride * 0] = r17;
  vout[span_r + span_stride * 1] = r18;
  vout[span_r + span_stride * 2] = r19;
  vout[span_r + span_stride * 3] = r20;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_fm_1_3(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (16 + 1) - span_off - 1;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_r + span_stride * 0];
  ulong r18 = vout[span_r + span_stride * 1];
  ulong r19 = vout[span_r + span_stride * 2];
  ulong r20 = vout[span_r + span_stride * 3];
  ulong r21 = vout[span_r + span_stride * 4];
  ulong r22 = vout[span_r + span_stride * 5];
  ulong r23 = vout[span_r + span_stride * 6];
  ulong r24 = vout[span_r + span_stride * 7];
  if (r16 >= r17) {
    ulong const t = r16;
    r16 = r17;
    r17 = t;
  };
  if (r15 >= r18) {
    ulong const t = r15;
    r15 = r18;
    r18 = t;
  };
  if (r14 >= r19) {
    ulong const t = r14;
    r14 = r19;
    r19 = t;
  };
  if (r13 >= r20) {
    ulong const t = r13;
    r13 = r20;
    r20 = t;
  };
  if (r12 >= r21) {
    ulong const t = r12;
    r12 = r21;
    r21 = t;
  };
  if (r11 >= r22) {
    ulong const t = r11;
    r11 = r22;
    r22 = t;
  };
  if (r10 >= r23) {
    ulong const t = r10;
    r10 = r23;
    r23 = t;
  };
  if (r9 >= r24) {
    ulong const t = r9;
    r9 = r24;
    r24 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r17 >= r21) {
    ulong const t = r17;
    r17 = r21;
    r21 = t;
  };
  if (r19 >= r23) {
    ulong const t = r19;
    r19 = r23;
    r23 = t;
  };
  if (r17 >= r19) {
    ulong const t = r17;
    r17 = r19;
    r19 = t;
  };
  if (r21 >= r23) {
    ulong const t = r21;
    r21 = r23;
    r23 = t;
  };
  if (r18 >= r22) {
    ulong const t = r18;
    r18 = r22;
    r22 = t;
  };
  if (r20 >= r24) {
    ulong const t = r20;
    r20 = r24;
    r24 = t;
  };
  if (r18 >= r20) {
    ulong const t = r18;
    r18 = r20;
    r20 = t;
  };
  if (r22 >= r24) {
    ulong const t = r22;
    r22 = r24;
    r24 = t;
  };
  if (r17 >= r18) {
    ulong const t = r17;
    r17 = r18;
    r18 = t;
  };
  if (r19 >= r20) {
    ulong const t = r19;
    r19 = r20;
    r20 = t;
  };
  if (r21 >= r22) {
    ulong const t = r21;
    r21 = r22;
    r22 = t;
  };
  if (r23 >= r24) {
    ulong const t = r23;
    r23 = r24;
    r24 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_r + span_stride * 0] = r17;
  vout[span_r + span_stride * 1] = r18;
  vout[span_r + span_stride * 2] = r19;
  vout[span_r + span_stride * 3] = r20;
  vout[span_r + span_stride * 4] = r21;
  vout[span_r + span_stride * 5] = r22;
  vout[span_r + span_stride * 6] = r23;
  vout[span_r + span_stride * 7] = r24;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_fm_1_4(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (16 + 1) - span_off - 1;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_r + span_stride * 0];
  ulong r18 = vout[span_r + span_stride * 1];
  ulong r19 = vout[span_r + span_stride * 2];
  ulong r20 = vout[span_r + span_stride * 3];
  ulong r21 = vout[span_r + span_stride * 4];
  ulong r22 = vout[span_r + span_stride * 5];
  ulong r23 = vout[span_r + span_stride * 6];
  ulong r24 = vout[span_r + span_stride * 7];
  ulong r25 = vout[span_r + span_stride * 8];
  ulong r26 = vout[span_r + span_stride * 9];
  ulong r27 = vout[span_r + span_stride * 10];
  ulong r28 = vout[span_r + span_stride * 11];
  ulong r29 = vout[span_r + span_stride * 12];
  ulong r30 = vout[span_r + span_stride * 13];
  ulong r31 = vout[span_r + span_stride * 14];
  ulong r32 = vout[span_r + span_stride * 15];
  if (r16 >= r17) {
    ulong const t = r16;
    r16 = r17;
    r17 = t;
  };
  if (r15 >= r18) {
    ulong const t = r15;
    r15 = r18;
    r18 = t;
  };
  if (r14 >= r19) {
    ulong const t = r14;
    r14 = r19;
    r19 = t;
  };
  if (r13 >= r20) {
    ulong const t = r13;
    r13 = r20;
    r20 = t;
  };
  if (r12 >= r21) {
    ulong const t = r12;
    r12 = r21;
    r21 = t;
  };
  if (r11 >= r22) {
    ulong const t = r11;
    r11 = r22;
    r22 = t;
  };
  if (r10 >= r23) {
    ulong const t = r10;
    r10 = r23;
    r23 = t;
  };
  if (r9 >= r24) {
    ulong const t = r9;
    r9 = r24;
    r24 = t;
  };
  if (r8 >= r25) {
    ulong const t = r8;
    r8 = r25;
    r25 = t;
  };
  if (r7 >= r26) {
    ulong const t = r7;
    r7 = r26;
    r26 = t;
  };
  if (r6 >= r27) {
    ulong const t = r6;
    r6 = r27;
    r27 = t;
  };
  if (r5 >= r28) {
    ulong const t = r5;
    r5 = r28;
    r28 = t;
  };
  if (r4 >= r29) {
    ulong const t = r4;
    r4 = r29;
    r29 = t;
  };
  if (r3 >= r30) {
    ulong const t = r3;
    r3 = r30;
    r30 = t;
  };
  if (r2 >= r31) {
    ulong const t = r2;
    r2 = r31;
    r31 = t;
  };
  if (r1 >= r32) {
    ulong const t = r1;
    r1 = r32;
    r32 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r17 >= r25) {
    ulong const t = r17;
    r17 = r25;
    r25 = t;
  };
  if (r21 >= r29) {
    ulong const t = r21;
    r21 = r29;
    r29 = t;
  };
  if (r17 >= r21) {
    ulong const t = r17;
    r17 = r21;
    r21 = t;
  };
  if (r25 >= r29) {
    ulong const t = r25;
    r25 = r29;
    r29 = t;
  };
  if (r19 >= r27) {
    ulong const t = r19;
    r19 = r27;
    r27 = t;
  };
  if (r23 >= r31) {
    ulong const t = r23;
    r23 = r31;
    r31 = t;
  };
  if (r19 >= r23) {
    ulong const t = r19;
    r19 = r23;
    r23 = t;
  };
  if (r27 >= r31) {
    ulong const t = r27;
    r27 = r31;
    r31 = t;
  };
  if (r17 >= r19) {
    ulong const t = r17;
    r17 = r19;
    r19 = t;
  };
  if (r21 >= r23) {
    ulong const t = r21;
    r21 = r23;
    r23 = t;
  };
  if (r25 >= r27) {
    ulong const t = r25;
    r25 = r27;
    r27 = t;
  };
  if (r29 >= r31) {
    ulong const t = r29;
    r29 = r31;
    r31 = t;
  };
  if (r18 >= r26) {
    ulong const t = r18;
    r18 = r26;
    r26 = t;
  };
  if (r22 >= r30) {
    ulong const t = r22;
    r22 = r30;
    r30 = t;
  };
  if (r18 >= r22) {
    ulong const t = r18;
    r18 = r22;
    r22 = t;
  };
  if (r26 >= r30) {
    ulong const t = r26;
    r26 = r30;
    r30 = t;
  };
  if (r20 >= r28) {
    ulong const t = r20;
    r20 = r28;
    r28 = t;
  };
  if (r24 >= r32) {
    ulong const t = r24;
    r24 = r32;
    r32 = t;
  };
  if (r20 >= r24) {
    ulong const t = r20;
    r20 = r24;
    r24 = t;
  };
  if (r28 >= r32) {
    ulong const t = r28;
    r28 = r32;
    r32 = t;
  };
  if (r18 >= r20) {
    ulong const t = r18;
    r18 = r20;
    r20 = t;
  };
  if (r22 >= r24) {
    ulong const t = r22;
    r22 = r24;
    r24 = t;
  };
  if (r26 >= r28) {
    ulong const t = r26;
    r26 = r28;
    r28 = t;
  };
  if (r30 >= r32) {
    ulong const t = r30;
    r30 = r32;
    r32 = t;
  };
  if (r17 >= r18) {
    ulong const t = r17;
    r17 = r18;
    r18 = t;
  };
  if (r19 >= r20) {
    ulong const t = r19;
    r19 = r20;
    r20 = t;
  };
  if (r21 >= r22) {
    ulong const t = r21;
    r21 = r22;
    r22 = t;
  };
  if (r23 >= r24) {
    ulong const t = r23;
    r23 = r24;
    r24 = t;
  };
  if (r25 >= r26) {
    ulong const t = r25;
    r25 = r26;
    r26 = t;
  };
  if (r27 >= r28) {
    ulong const t = r27;
    r27 = r28;
    r28 = t;
  };
  if (r29 >= r30) {
    ulong const t = r29;
    r29 = r30;
    r30 = t;
  };
  if (r31 >= r32) {
    ulong const t = r31;
    r31 = r32;
    r32 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_r + span_stride * 0] = r17;
  vout[span_r + span_stride * 1] = r18;
  vout[span_r + span_stride * 2] = r19;
  vout[span_r + span_stride * 3] = r20;
  vout[span_r + span_stride * 4] = r21;
  vout[span_r + span_stride * 5] = r22;
  vout[span_r + span_stride * 6] = r23;
  vout[span_r + span_stride * 7] = r24;
  vout[span_r + span_stride * 8] = r25;
  vout[span_r + span_stride * 9] = r26;
  vout[span_r + span_stride * 10] = r27;
  vout[span_r + span_stride * 11] = r28;
  vout[span_r + span_stride * 12] = r29;
  vout[span_r + span_stride * 13] = r30;
  vout[span_r + span_stride * 14] = r31;
  vout[span_r + span_stride * 15] = r32;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_hm_1(__global ulong* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 16 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  ulong r1 = vout[span_l + span_stride * 0];
  ulong r2 = vout[span_l + span_stride * 1];
  ulong r3 = vout[span_l + span_stride * 2];
  ulong r4 = vout[span_l + span_stride * 3];
  ulong r5 = vout[span_l + span_stride * 4];
  ulong r6 = vout[span_l + span_stride * 5];
  ulong r7 = vout[span_l + span_stride * 6];
  ulong r8 = vout[span_l + span_stride * 7];
  ulong r9 = vout[span_l + span_stride * 8];
  ulong r10 = vout[span_l + span_stride * 9];
  ulong r11 = vout[span_l + span_stride * 10];
  ulong r12 = vout[span_l + span_stride * 11];
  ulong r13 = vout[span_l + span_stride * 12];
  ulong r14 = vout[span_l + span_stride * 13];
  ulong r15 = vout[span_l + span_stride * 14];
  ulong r16 = vout[span_l + span_stride * 15];
  ulong r17 = vout[span_l + span_stride * 16];
  ulong r18 = vout[span_l + span_stride * 17];
  ulong r19 = vout[span_l + span_stride * 18];
  ulong r20 = vout[span_l + span_stride * 19];
  ulong r21 = vout[span_l + span_stride * 20];
  ulong r22 = vout[span_l + span_stride * 21];
  ulong r23 = vout[span_l + span_stride * 22];
  ulong r24 = vout[span_l + span_stride * 23];
  ulong r25 = vout[span_l + span_stride * 24];
  ulong r26 = vout[span_l + span_stride * 25];
  ulong r27 = vout[span_l + span_stride * 26];
  ulong r28 = vout[span_l + span_stride * 27];
  ulong r29 = vout[span_l + span_stride * 28];
  ulong r30 = vout[span_l + span_stride * 29];
  ulong r31 = vout[span_l + span_stride * 30];
  ulong r32 = vout[span_l + span_stride * 31];
  if (r1 >= r17) {
    ulong const t = r1;
    r1 = r17;
    r17 = t;
  };
  if (r9 >= r25) {
    ulong const t = r9;
    r9 = r25;
    r25 = t;
  };
  if (r1 >= r9) {
    ulong const t = r1;
    r1 = r9;
    r9 = t;
  };
  if (r17 >= r25) {
    ulong const t = r17;
    r17 = r25;
    r25 = t;
  };
  if (r5 >= r21) {
    ulong const t = r5;
    r5 = r21;
    r21 = t;
  };
  if (r13 >= r29) {
    ulong const t = r13;
    r13 = r29;
    r29 = t;
  };
  if (r5 >= r13) {
    ulong const t = r5;
    r5 = r13;
    r13 = t;
  };
  if (r21 >= r29) {
    ulong const t = r21;
    r21 = r29;
    r29 = t;
  };
  if (r1 >= r5) {
    ulong const t = r1;
    r1 = r5;
    r5 = t;
  };
  if (r9 >= r13) {
    ulong const t = r9;
    r9 = r13;
    r13 = t;
  };
  if (r17 >= r21) {
    ulong const t = r17;
    r17 = r21;
    r21 = t;
  };
  if (r25 >= r29) {
    ulong const t = r25;
    r25 = r29;
    r29 = t;
  };
  if (r3 >= r19) {
    ulong const t = r3;
    r3 = r19;
    r19 = t;
  };
  if (r11 >= r27) {
    ulong const t = r11;
    r11 = r27;
    r27 = t;
  };
  if (r3 >= r11) {
    ulong const t = r3;
    r3 = r11;
    r11 = t;
  };
  if (r19 >= r27) {
    ulong const t = r19;
    r19 = r27;
    r27 = t;
  };
  if (r7 >= r23) {
    ulong const t = r7;
    r7 = r23;
    r23 = t;
  };
  if (r15 >= r31) {
    ulong const t = r15;
    r15 = r31;
    r31 = t;
  };
  if (r7 >= r15) {
    ulong const t = r7;
    r7 = r15;
    r15 = t;
  };
  if (r23 >= r31) {
    ulong const t = r23;
    r23 = r31;
    r31 = t;
  };
  if (r3 >= r7) {
    ulong const t = r3;
    r3 = r7;
    r7 = t;
  };
  if (r11 >= r15) {
    ulong const t = r11;
    r11 = r15;
    r15 = t;
  };
  if (r19 >= r23) {
    ulong const t = r19;
    r19 = r23;
    r23 = t;
  };
  if (r27 >= r31) {
    ulong const t = r27;
    r27 = r31;
    r31 = t;
  };
  if (r1 >= r3) {
    ulong const t = r1;
    r1 = r3;
    r3 = t;
  };
  if (r5 >= r7) {
    ulong const t = r5;
    r5 = r7;
    r7 = t;
  };
  if (r9 >= r11) {
    ulong const t = r9;
    r9 = r11;
    r11 = t;
  };
  if (r13 >= r15) {
    ulong const t = r13;
    r13 = r15;
    r15 = t;
  };
  if (r17 >= r19) {
    ulong const t = r17;
    r17 = r19;
    r19 = t;
  };
  if (r21 >= r23) {
    ulong const t = r21;
    r21 = r23;
    r23 = t;
  };
  if (r25 >= r27) {
    ulong const t = r25;
    r25 = r27;
    r27 = t;
  };
  if (r29 >= r31) {
    ulong const t = r29;
    r29 = r31;
    r31 = t;
  };
  if (r2 >= r18) {
    ulong const t = r2;
    r2 = r18;
    r18 = t;
  };
  if (r10 >= r26) {
    ulong const t = r10;
    r10 = r26;
    r26 = t;
  };
  if (r2 >= r10) {
    ulong const t = r2;
    r2 = r10;
    r10 = t;
  };
  if (r18 >= r26) {
    ulong const t = r18;
    r18 = r26;
    r26 = t;
  };
  if (r6 >= r22) {
    ulong const t = r6;
    r6 = r22;
    r22 = t;
  };
  if (r14 >= r30) {
    ulong const t = r14;
    r14 = r30;
    r30 = t;
  };
  if (r6 >= r14) {
    ulong const t = r6;
    r6 = r14;
    r14 = t;
  };
  if (r22 >= r30) {
    ulong const t = r22;
    r22 = r30;
    r30 = t;
  };
  if (r2 >= r6) {
    ulong const t = r2;
    r2 = r6;
    r6 = t;
  };
  if (r10 >= r14) {
    ulong const t = r10;
    r10 = r14;
    r14 = t;
  };
  if (r18 >= r22) {
    ulong const t = r18;
    r18 = r22;
    r22 = t;
  };
  if (r26 >= r30) {
    ulong const t = r26;
    r26 = r30;
    r30 = t;
  };
  if (r4 >= r20) {
    ulong const t = r4;
    r4 = r20;
    r20 = t;
  };
  if (r12 >= r28) {
    ulong const t = r12;
    r12 = r28;
    r28 = t;
  };
  if (r4 >= r12) {
    ulong const t = r4;
    r4 = r12;
    r12 = t;
  };
  if (r20 >= r28) {
    ulong const t = r20;
    r20 = r28;
    r28 = t;
  };
  if (r8 >= r24) {
    ulong const t = r8;
    r8 = r24;
    r24 = t;
  };
  if (r16 >= r32) {
    ulong const t = r16;
    r16 = r32;
    r32 = t;
  };
  if (r8 >= r16) {
    ulong const t = r8;
    r8 = r16;
    r16 = t;
  };
  if (r24 >= r32) {
    ulong const t = r24;
    r24 = r32;
    r32 = t;
  };
  if (r4 >= r8) {
    ulong const t = r4;
    r4 = r8;
    r8 = t;
  };
  if (r12 >= r16) {
    ulong const t = r12;
    r12 = r16;
    r16 = t;
  };
  if (r20 >= r24) {
    ulong const t = r20;
    r20 = r24;
    r24 = t;
  };
  if (r28 >= r32) {
    ulong const t = r28;
    r28 = r32;
    r32 = t;
  };
  if (r2 >= r4) {
    ulong const t = r2;
    r2 = r4;
    r4 = t;
  };
  if (r6 >= r8) {
    ulong const t = r6;
    r6 = r8;
    r8 = t;
  };
  if (r10 >= r12) {
    ulong const t = r10;
    r10 = r12;
    r12 = t;
  };
  if (r14 >= r16) {
    ulong const t = r14;
    r14 = r16;
    r16 = t;
  };
  if (r18 >= r20) {
    ulong const t = r18;
    r18 = r20;
    r20 = t;
  };
  if (r22 >= r24) {
    ulong const t = r22;
    r22 = r24;
    r24 = t;
  };
  if (r26 >= r28) {
    ulong const t = r26;
    r26 = r28;
    r28 = t;
  };
  if (r30 >= r32) {
    ulong const t = r30;
    r30 = r32;
    r32 = t;
  };
  if (r1 >= r2) {
    ulong const t = r1;
    r1 = r2;
    r2 = t;
  };
  if (r3 >= r4) {
    ulong const t = r3;
    r3 = r4;
    r4 = t;
  };
  if (r5 >= r6) {
    ulong const t = r5;
    r5 = r6;
    r6 = t;
  };
  if (r7 >= r8) {
    ulong const t = r7;
    r7 = r8;
    r8 = t;
  };
  if (r9 >= r10) {
    ulong const t = r9;
    r9 = r10;
    r10 = t;
  };
  if (r11 >= r12) {
    ulong const t = r11;
    r11 = r12;
    r12 = t;
  };
  if (r13 >= r14) {
    ulong const t = r13;
    r13 = r14;
    r14 = t;
  };
  if (r15 >= r16) {
    ulong const t = r15;
    r15 = r16;
    r16 = t;
  };
  if (r17 >= r18) {
    ulong const t = r17;
    r17 = r18;
    r18 = t;
  };
  if (r19 >= r20) {
    ulong const t = r19;
    r19 = r20;
    r20 = t;
  };
  if (r21 >= r22) {
    ulong const t = r21;
    r21 = r22;
    r22 = t;
  };
  if (r23 >= r24) {
    ulong const t = r23;
    r23 = r24;
    r24 = t;
  };
  if (r25 >= r26) {
    ulong const t = r25;
    r25 = r26;
    r26 = t;
  };
  if (r27 >= r28) {
    ulong const t = r27;
    r27 = r28;
    r28 = t;
  };
  if (r29 >= r30) {
    ulong const t = r29;
    r29 = r30;
    r30 = t;
  };
  if (r31 >= r32) {
    ulong const t = r31;
    r31 = r32;
    r32 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_l + span_stride * 8] = r9;
  vout[span_l + span_stride * 9] = r10;
  vout[span_l + span_stride * 10] = r11;
  vout[span_l + span_stride * 11] = r12;
  vout[span_l + span_stride * 12] = r13;
  vout[span_l + span_stride * 13] = r14;
  vout[span_l + span_stride * 14] = r15;
  vout[span_l + span_stride * 15] = r16;
  vout[span_l + span_stride * 16] = r17;
  vout[span_l + span_stride * 17] = r18;
  vout[span_l + span_stride * 18] = r19;
  vout[span_l + span_stride * 19] = r20;
  vout[span_l + span_stride * 20] = r21;
  vout[span_l + span_stride * 21] = r22;
  vout[span_l + span_stride * 22] = r23;
  vout[span_l + span_stride * 23] = r24;
  vout[span_l + span_stride * 24] = r25;
  vout[span_l + span_stride * 25] = r26;
  vout[span_l + span_stride * 26] = r27;
  vout[span_l + span_stride * 27] = r28;
  vout[span_l + span_stride * 28] = r29;
  vout[span_l + span_stride * 29] = r30;
  vout[span_l + span_stride * 30] = r31;
  vout[span_l + span_stride * 31] = r32;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 3)))) void
hs_kernel_transpose(__global ulong* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 3) - 1)) * 16 +
                        (get_local_id(0) & ((1 << 3) - 1));
  ulong r1 = vout[gmem_idx + (1 << 3) * 0];
  ulong r2 = vout[gmem_idx + (1 << 3) * 1];
  ulong r3 = vout[gmem_idx + (1 << 3) * 2];
  ulong r4 = vout[gmem_idx + (1 << 3) * 3];
  ulong r5 = vout[gmem_idx + (1 << 3) * 4];
  ulong r6 = vout[gmem_idx + (1 << 3) * 5];
  ulong r7 = vout[gmem_idx + (1 << 3) * 6];
  ulong r8 = vout[gmem_idx + (1 << 3) * 7];
  ulong r9 = vout[gmem_idx + (1 << 3) * 8];
  ulong r10 = vout[gmem_idx + (1 << 3) * 9];
  ulong r11 = vout[gmem_idx + (1 << 3) * 10];
  ulong r12 = vout[gmem_idx + (1 << 3) * 11];
  ulong r13 = vout[gmem_idx + (1 << 3) * 12];
  ulong r14 = vout[gmem_idx + (1 << 3) * 13];
  ulong r15 = vout[gmem_idx + (1 << 3) * 14];
  ulong r16 = vout[gmem_idx + (1 << 3) * 15];
  bool const is_lo_1 = (get_sub_group_local_id() & (1 << (1 - 1))) == 0;
  bool const is_lo_2 = (get_sub_group_local_id() & (1 << (2 - 1))) == 0;
  bool const is_lo_3 = (get_sub_group_local_id() & (1 << (3 - 1))) == 0;
  ulong const s2_1 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r2 : r1, 1 << (1 - 1));
  ulong const s2 = is_lo_1 ? s2_1 : r2;
  ulong const s1 = is_lo_1 ? r1 : s2_1;
  ulong const s4_3 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r4 : r3, 1 << (1 - 1));
  ulong const s4 = is_lo_1 ? s4_3 : r4;
  ulong const s3 = is_lo_1 ? r3 : s4_3;
  ulong const s6_5 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r6 : r5, 1 << (1 - 1));
  ulong const s6 = is_lo_1 ? s6_5 : r6;
  ulong const s5 = is_lo_1 ? r5 : s6_5;
  ulong const s8_7 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r8 : r7, 1 << (1 - 1));
  ulong const s8 = is_lo_1 ? s8_7 : r8;
  ulong const s7 = is_lo_1 ? r7 : s8_7;
  ulong const s10_9 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r10 : r9, 1 << (1 - 1));
  ulong const s10 = is_lo_1 ? s10_9 : r10;
  ulong const s9 = is_lo_1 ? r9 : s10_9;
  ulong const s12_11 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r12 : r11, 1 << (1 - 1));
  ulong const s12 = is_lo_1 ? s12_11 : r12;
  ulong const s11 = is_lo_1 ? r11 : s12_11;
  ulong const s14_13 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r14 : r13, 1 << (1 - 1));
  ulong const s14 = is_lo_1 ? s14_13 : r14;
  ulong const s13 = is_lo_1 ? r13 : s14_13;
  ulong const s16_15 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r16 : r15, 1 << (1 - 1));
  ulong const s16 = is_lo_1 ? s16_15 : r16;
  ulong const s15 = is_lo_1 ? r15 : s16_15;
  ulong const t3_1 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s3 : s1, 1 << (2 - 1));
  ulong const t3 = is_lo_2 ? t3_1 : s3;
  ulong const t1 = is_lo_2 ? s1 : t3_1;
  ulong const t4_2 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s4 : s2, 1 << (2 - 1));
  ulong const t4 = is_lo_2 ? t4_2 : s4;
  ulong const t2 = is_lo_2 ? s2 : t4_2;
  ulong const t7_5 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s7 : s5, 1 << (2 - 1));
  ulong const t7 = is_lo_2 ? t7_5 : s7;
  ulong const t5 = is_lo_2 ? s5 : t7_5;
  ulong const t8_6 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s8 : s6, 1 << (2 - 1));
  ulong const t8 = is_lo_2 ? t8_6 : s8;
  ulong const t6 = is_lo_2 ? s6 : t8_6;
  ulong const t11_9 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s11 : s9, 1 << (2 - 1));
  ulong const t11 = is_lo_2 ? t11_9 : s11;
  ulong const t9 = is_lo_2 ? s9 : t11_9;
  ulong const t12_10 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s12 : s10, 1 << (2 - 1));
  ulong const t12 = is_lo_2 ? t12_10 : s12;
  ulong const t10 = is_lo_2 ? s10 : t12_10;
  ulong const t15_13 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s15 : s13, 1 << (2 - 1));
  ulong const t15 = is_lo_2 ? t15_13 : s15;
  ulong const t13 = is_lo_2 ? s13 : t15_13;
  ulong const t16_14 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s16 : s14, 1 << (2 - 1));
  ulong const t16 = is_lo_2 ? t16_14 : s16;
  ulong const t14 = is_lo_2 ? s14 : t16_14;
  ulong const u5_1 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t5 : t1, 1 << (3 - 1));
  ulong const u5 = is_lo_3 ? u5_1 : t5;
  ulong const u1 = is_lo_3 ? t1 : u5_1;
  ulong const u6_2 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t6 : t2, 1 << (3 - 1));
  ulong const u6 = is_lo_3 ? u6_2 : t6;
  ulong const u2 = is_lo_3 ? t2 : u6_2;
  ulong const u7_3 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t7 : t3, 1 << (3 - 1));
  ulong const u7 = is_lo_3 ? u7_3 : t7;
  ulong const u3 = is_lo_3 ? t3 : u7_3;
  ulong const u8_4 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t8 : t4, 1 << (3 - 1));
  ulong const u8 = is_lo_3 ? u8_4 : t8;
  ulong const u4 = is_lo_3 ? t4 : u8_4;
  ulong const u13_9 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t13 : t9, 1 << (3 - 1));
  ulong const u13 = is_lo_3 ? u13_9 : t13;
  ulong const u9 = is_lo_3 ? t9 : u13_9;
  ulong const u14_10 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t14 : t10, 1 << (3 - 1));
  ulong const u14 = is_lo_3 ? u14_10 : t14;
  ulong const u10 = is_lo_3 ? t10 : u14_10;
  ulong const u15_11 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t15 : t11, 1 << (3 - 1));
  ulong const u15 = is_lo_3 ? u15_11 : t15;
  ulong const u11 = is_lo_3 ? t11 : u15_11;
  ulong const u16_12 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t16 : t12, 1 << (3 - 1));
  ulong const u16 = is_lo_3 ? u16_12 : t16;
  ulong const u12 = is_lo_3 ? t12 : u16_12;
  vout[gmem_idx + ((1 - 1) << 3)] = u1;
  vout[gmem_idx + ((3 - 1) << 3)] = u2;
  vout[gmem_idx + ((5 - 1) << 3)] = u3;
  vout[gmem_idx + ((7 - 1) << 3)] = u4;
  vout[gmem_idx + ((9 - 1) << 3)] = u5;
  vout[gmem_idx + ((11 - 1) << 3)] = u6;
  vout[gmem_idx + ((13 - 1) << 3)] = u7;
  vout[gmem_idx + ((15 - 1) << 3)] = u8;
  vout[gmem_idx + ((2 - 1) << 3)] = u9;
  vout[gmem_idx + ((4 - 1) << 3)] = u10;
  vout[gmem_idx + ((6 - 1) << 3)] = u11;
  vout[gmem_idx + ((8 - 1) << 3)] = u12;
  vout[gmem_idx + ((10 - 1) << 3)] = u13;
  vout[gmem_idx + ((12 - 1) << 3)] = u14;
  vout[gmem_idx + ((14 - 1) << 3)] = u15;
  vout[gmem_idx + ((16 - 1) << 3)] = u16;
}
