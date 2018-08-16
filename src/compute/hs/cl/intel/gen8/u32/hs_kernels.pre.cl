

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 1, 1, 1))) void
hs_kernel_bs_0(__global uint const* const restrict vin,
               __global uint* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vin[gmem_idx + (1 << 4) * 0];
  uint r2 = vin[gmem_idx + (1 << 4) * 1];
  uint r3 = vin[gmem_idx + (1 << 4) * 2];
  uint r4 = vin[gmem_idx + (1 << 4) * 3];
  uint r5 = vin[gmem_idx + (1 << 4) * 4];
  uint r6 = vin[gmem_idx + (1 << 4) * 5];
  uint r7 = vin[gmem_idx + (1 << 4) * 6];
  uint r8 = vin[gmem_idx + (1 << 4) * 7];
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r3, r5);
    r5 = max(r3, r5);
    r3 = t;
  };
  {
    uint const t = min(r4, r6);
    r6 = max(r4, r6);
    r4 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r2, r5);
    r5 = max(r2, r5);
    r2 = t;
  };
  {
    uint const t = min(r4, r7);
    r7 = max(r4, r7);
    r4 = t;
  };
  {
    uint const t = min(r2, r3);
    r3 = max(r2, r3);
    r2 = t;
  };
  {
    uint const t = min(r4, r5);
    r5 = max(r4, r5);
    r4 = t;
  };
  {
    uint const t = min(r6, r7);
    r7 = max(r6, r7);
    r6 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 15;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 4;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 2, 1, 1))) void
hs_kernel_bs_1(__global uint const* const restrict vin,
               __global uint* const restrict vout)
{
  __local struct
  {
    uint m[32 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vin[gmem_idx + (1 << 4) * 0];
  uint r2 = vin[gmem_idx + (1 << 4) * 1];
  uint r3 = vin[gmem_idx + (1 << 4) * 2];
  uint r4 = vin[gmem_idx + (1 << 4) * 3];
  uint r5 = vin[gmem_idx + (1 << 4) * 4];
  uint r6 = vin[gmem_idx + (1 << 4) * 5];
  uint r7 = vin[gmem_idx + (1 << 4) * 6];
  uint r8 = vin[gmem_idx + (1 << 4) * 7];
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r3, r5);
    r5 = max(r3, r5);
    r3 = t;
  };
  {
    uint const t = min(r4, r6);
    r6 = max(r4, r6);
    r4 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r2, r5);
    r5 = max(r2, r5);
    r2 = t;
  };
  {
    uint const t = min(r4, r7);
    r7 = max(r4, r7);
    r4 = t;
  };
  {
    uint const t = min(r2, r3);
    r3 = max(r2, r3);
    r2 = t;
  };
  {
    uint const t = min(r4, r5);
    r5 = max(r4, r5);
    r4 = t;
  };
  {
    uint const t = min(r6, r7);
    r7 = max(r6, r7);
    r6 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 15;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 4;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 2) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 4) * 2) +
                          (get_sub_group_local_id() ^ ((1 << 4) - 1));
  shared.m[get_local_id(0) + (2 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (2 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_r_idx + (16)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (16)] = r0_2;
    }
    {
      uint r0_1 = shared.m[smem_l_idx + (64)];
      uint r0_2 = shared.m[smem_r_idx + (80)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r0_1;
      shared.m[smem_r_idx + (80)] = r0_2;
    }
    {
      uint r0_1 = shared.m[smem_l_idx + (128)];
      uint r0_2 = shared.m[smem_r_idx + (144)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_r_idx + (144)] = r0_2;
    }
    {
      uint r0_1 = shared.m[smem_l_idx + (192)];
      uint r0_2 = shared.m[smem_r_idx + (208)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (192)] = r0_1;
      shared.m[smem_r_idx + (208)] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (2 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (2 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (2 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (2 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (2 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (2 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (2 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (2 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 4, 1, 1))) void
hs_kernel_bs_2(__global uint const* const restrict vin,
               __global uint* const restrict vout)
{
  __local struct
  {
    uint m[64 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vin[gmem_idx + (1 << 4) * 0];
  uint r2 = vin[gmem_idx + (1 << 4) * 1];
  uint r3 = vin[gmem_idx + (1 << 4) * 2];
  uint r4 = vin[gmem_idx + (1 << 4) * 3];
  uint r5 = vin[gmem_idx + (1 << 4) * 4];
  uint r6 = vin[gmem_idx + (1 << 4) * 5];
  uint r7 = vin[gmem_idx + (1 << 4) * 6];
  uint r8 = vin[gmem_idx + (1 << 4) * 7];
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r3, r5);
    r5 = max(r3, r5);
    r3 = t;
  };
  {
    uint const t = min(r4, r6);
    r6 = max(r4, r6);
    r4 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r2, r5);
    r5 = max(r2, r5);
    r2 = t;
  };
  {
    uint const t = min(r4, r7);
    r7 = max(r4, r7);
    r4 = t;
  };
  {
    uint const t = min(r2, r3);
    r3 = max(r2, r3);
    r2 = t;
  };
  {
    uint const t = min(r4, r5);
    r5 = max(r4, r5);
    r4 = t;
  };
  {
    uint const t = min(r6, r7);
    r7 = max(r6, r7);
    r6 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 15;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 4;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 4) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 4) * 4) +
                          (get_sub_group_local_id() ^ ((1 << 4) - 1));
  shared.m[get_local_id(0) + (4 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_r_idx + (16)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (16)] = r0_2;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (32)];
      uint r1_2 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (32)] = r1_1;
      shared.m[smem_r_idx + (48)] = r1_2;
    }
    {
      uint r0_1 = shared.m[smem_l_idx + (256)];
      uint r0_2 = shared.m[smem_r_idx + (272)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_r_idx + (272)] = r0_2;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (288)];
      uint r1_2 = shared.m[smem_r_idx + (304)];
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (288)] = r1_1;
      shared.m[smem_r_idx + (304)] = r1_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (4 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (4 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (4 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (4 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (4 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (4 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (4 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (4 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (4 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (4 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_r_idx + (32)];
      uint r0_4 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r0_2, r0_3);
        r0_3 = max(r0_2, r0_3);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_4);
        r0_4 = max(r0_1, r0_4);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_r_idx + (32)] = r0_3;
      shared.m[smem_r_idx + (48)] = r0_4;
    }
    {
      uint r0_1 = shared.m[smem_l_idx + (256)];
      uint r0_2 = shared.m[smem_l_idx + (272)];
      uint r0_3 = shared.m[smem_r_idx + (288)];
      uint r0_4 = shared.m[smem_r_idx + (304)];
      {
        uint const t = min(r0_2, r0_3);
        r0_3 = max(r0_2, r0_3);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_4);
        r0_4 = max(r0_1, r0_4);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_l_idx + (272)] = r0_2;
      shared.m[smem_r_idx + (288)] = r0_3;
      shared.m[smem_r_idx + (304)] = r0_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (4 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (4 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (4 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (4 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (4 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (4 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (4 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (4 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 8, 1, 1))) void
hs_kernel_bs_3(__global uint const* const restrict vin,
               __global uint* const restrict vout)
{
  __local struct
  {
    uint m[128 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vin[gmem_idx + (1 << 4) * 0];
  uint r2 = vin[gmem_idx + (1 << 4) * 1];
  uint r3 = vin[gmem_idx + (1 << 4) * 2];
  uint r4 = vin[gmem_idx + (1 << 4) * 3];
  uint r5 = vin[gmem_idx + (1 << 4) * 4];
  uint r6 = vin[gmem_idx + (1 << 4) * 5];
  uint r7 = vin[gmem_idx + (1 << 4) * 6];
  uint r8 = vin[gmem_idx + (1 << 4) * 7];
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r3, r5);
    r5 = max(r3, r5);
    r3 = t;
  };
  {
    uint const t = min(r4, r6);
    r6 = max(r4, r6);
    r4 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r2, r5);
    r5 = max(r2, r5);
    r2 = t;
  };
  {
    uint const t = min(r4, r7);
    r7 = max(r4, r7);
    r4 = t;
  };
  {
    uint const t = min(r2, r3);
    r3 = max(r2, r3);
    r2 = t;
  };
  {
    uint const t = min(r4, r5);
    r5 = max(r4, r5);
    r4 = t;
  };
  {
    uint const t = min(r6, r7);
    r7 = max(r6, r7);
    r6 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 15;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 4;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 8) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 4) * 8) +
                          (get_sub_group_local_id() ^ ((1 << 4) - 1));
  shared.m[get_local_id(0) + (8 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_r_idx + (16)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (16)] = r0_2;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (32)];
      uint r1_2 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (32)] = r1_1;
      shared.m[smem_r_idx + (48)] = r1_2;
    }
    {
      uint r2_1 = shared.m[smem_l_idx + (64)];
      uint r2_2 = shared.m[smem_r_idx + (80)];
      {
        uint const t = min(r2_1, r2_2);
        r2_2 = max(r2_1, r2_2);
        r2_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r2_1;
      shared.m[smem_r_idx + (80)] = r2_2;
    }
    {
      uint r3_1 = shared.m[smem_l_idx + (96)];
      uint r3_2 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r3_1, r3_2);
        r3_2 = max(r3_1, r3_2);
        r3_1 = t;
      };
      shared.m[smem_l_idx + (96)] = r3_1;
      shared.m[smem_r_idx + (112)] = r3_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (8 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_r_idx + (32)];
      uint r0_4 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r0_2, r0_3);
        r0_3 = max(r0_2, r0_3);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_4);
        r0_4 = max(r0_1, r0_4);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_r_idx + (32)] = r0_3;
      shared.m[smem_r_idx + (48)] = r0_4;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (64)];
      uint r1_2 = shared.m[smem_l_idx + (80)];
      uint r1_3 = shared.m[smem_r_idx + (96)];
      uint r1_4 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r1_2, r1_3);
        r1_3 = max(r1_2, r1_3);
        r1_2 = t;
      };
      {
        uint const t = min(r1_1, r1_4);
        r1_4 = max(r1_1, r1_4);
        r1_1 = t;
      };
      {
        uint const t = min(r1_3, r1_4);
        r1_4 = max(r1_3, r1_4);
        r1_3 = t;
      };
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r1_1;
      shared.m[smem_l_idx + (80)] = r1_2;
      shared.m[smem_r_idx + (96)] = r1_3;
      shared.m[smem_r_idx + (112)] = r1_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (8 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (8 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_l_idx + (32)];
      uint r0_4 = shared.m[smem_l_idx + (48)];
      uint r0_5 = shared.m[smem_r_idx + (64)];
      uint r0_6 = shared.m[smem_r_idx + (80)];
      uint r0_7 = shared.m[smem_r_idx + (96)];
      uint r0_8 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r0_4, r0_5);
        r0_5 = max(r0_4, r0_5);
        r0_4 = t;
      };
      {
        uint const t = min(r0_3, r0_6);
        r0_6 = max(r0_3, r0_6);
        r0_3 = t;
      };
      {
        uint const t = min(r0_2, r0_7);
        r0_7 = max(r0_2, r0_7);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_8);
        r0_8 = max(r0_1, r0_8);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_7);
        r0_7 = max(r0_5, r0_7);
        r0_5 = t;
      };
      {
        uint const t = min(r0_6, r0_8);
        r0_8 = max(r0_6, r0_8);
        r0_6 = t;
      };
      {
        uint const t = min(r0_5, r0_6);
        r0_6 = max(r0_5, r0_6);
        r0_5 = t;
      };
      {
        uint const t = min(r0_7, r0_8);
        r0_8 = max(r0_7, r0_8);
        r0_7 = t;
      };
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
      shared.m[smem_r_idx + (64)] = r0_5;
      shared.m[smem_r_idx + (80)] = r0_6;
      shared.m[smem_r_idx + (96)] = r0_7;
      shared.m[smem_r_idx + (112)] = r0_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (8 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (8 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (8 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (8 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (8 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (8 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (8 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (8 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 16, 1, 1))) void
hs_kernel_bs_4(__global uint const* const restrict vin,
               __global uint* const restrict vout)
{
  __local struct
  {
    uint m[256 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vin[gmem_idx + (1 << 4) * 0];
  uint r2 = vin[gmem_idx + (1 << 4) * 1];
  uint r3 = vin[gmem_idx + (1 << 4) * 2];
  uint r4 = vin[gmem_idx + (1 << 4) * 3];
  uint r5 = vin[gmem_idx + (1 << 4) * 4];
  uint r6 = vin[gmem_idx + (1 << 4) * 5];
  uint r7 = vin[gmem_idx + (1 << 4) * 6];
  uint r8 = vin[gmem_idx + (1 << 4) * 7];
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r3, r5);
    r5 = max(r3, r5);
    r3 = t;
  };
  {
    uint const t = min(r4, r6);
    r6 = max(r4, r6);
    r4 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r2, r5);
    r5 = max(r2, r5);
    r2 = t;
  };
  {
    uint const t = min(r4, r7);
    r7 = max(r4, r7);
    r4 = t;
  };
  {
    uint const t = min(r2, r3);
    r3 = max(r2, r3);
    r2 = t;
  };
  {
    uint const t = min(r4, r5);
    r5 = max(r4, r5);
    r4 = t;
  };
  {
    uint const t = min(r6, r7);
    r7 = max(r6, r7);
    r6 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 3;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 7;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const flip_lane_idx = get_sub_group_local_id() ^ 15;
    int const t_lt = get_sub_group_local_id() < flip_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r8, flip_lane_idx);
      r1 = ((r1 <= tb) ^ t_lt) ? tb : r1;
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r7, flip_lane_idx);
      r2 = ((r2 <= tb) ^ t_lt) ? tb : r2;
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r6, flip_lane_idx);
      r3 = ((r3 <= tb) ^ t_lt) ? tb : r3;
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, flip_lane_idx);
      uint const tb = intel_sub_group_shuffle(r5, flip_lane_idx);
      r4 = ((r4 <= tb) ^ t_lt) ? tb : r4;
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 4;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 2;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const half_lane_idx = get_sub_group_local_id() ^ 1;
    int const t_lt = get_sub_group_local_id() < half_lane_idx;
    ;
    {
      uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
      r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
    };
    {
      uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
      r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
    };
    {
      uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
      r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
    };
    {
      uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
      r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
    };
    {
      uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
      r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
    };
    {
      uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
      r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
    };
    {
      uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
      r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
    };
    {
      uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
      r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
    };
  }
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 16) + get_sub_group_local_id();
  uint const smem_r_idx = (get_sub_group_id() ^ 1) * ((1 << 4) * 16) +
                          (get_sub_group_local_id() ^ ((1 << 4) - 1));
  shared.m[get_local_id(0) + (16 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  if (get_sub_group_id() < 8) {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_r_idx + (16)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_r_idx + (16)] = r0_2;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (32)];
      uint r1_2 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (32)] = r1_1;
      shared.m[smem_r_idx + (48)] = r1_2;
    }
    {
      uint r2_1 = shared.m[smem_l_idx + (64)];
      uint r2_2 = shared.m[smem_r_idx + (80)];
      {
        uint const t = min(r2_1, r2_2);
        r2_2 = max(r2_1, r2_2);
        r2_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r2_1;
      shared.m[smem_r_idx + (80)] = r2_2;
    }
    {
      uint r3_1 = shared.m[smem_l_idx + (96)];
      uint r3_2 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r3_1, r3_2);
        r3_2 = max(r3_1, r3_2);
        r3_1 = t;
      };
      shared.m[smem_l_idx + (96)] = r3_1;
      shared.m[smem_r_idx + (112)] = r3_2;
    }
    {
      uint r4_1 = shared.m[smem_l_idx + (128)];
      uint r4_2 = shared.m[smem_r_idx + (144)];
      {
        uint const t = min(r4_1, r4_2);
        r4_2 = max(r4_1, r4_2);
        r4_1 = t;
      };
      shared.m[smem_l_idx + (128)] = r4_1;
      shared.m[smem_r_idx + (144)] = r4_2;
    }
    {
      uint r5_1 = shared.m[smem_l_idx + (160)];
      uint r5_2 = shared.m[smem_r_idx + (176)];
      {
        uint const t = min(r5_1, r5_2);
        r5_2 = max(r5_1, r5_2);
        r5_1 = t;
      };
      shared.m[smem_l_idx + (160)] = r5_1;
      shared.m[smem_r_idx + (176)] = r5_2;
    }
    {
      uint r6_1 = shared.m[smem_l_idx + (192)];
      uint r6_2 = shared.m[smem_r_idx + (208)];
      {
        uint const t = min(r6_1, r6_2);
        r6_2 = max(r6_1, r6_2);
        r6_1 = t;
      };
      shared.m[smem_l_idx + (192)] = r6_1;
      shared.m[smem_r_idx + (208)] = r6_2;
    }
    {
      uint r7_1 = shared.m[smem_l_idx + (224)];
      uint r7_2 = shared.m[smem_r_idx + (240)];
      {
        uint const t = min(r7_1, r7_2);
        r7_2 = max(r7_1, r7_2);
        r7_1 = t;
      };
      shared.m[smem_l_idx + (224)] = r7_1;
      shared.m[smem_r_idx + (240)] = r7_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  if (get_sub_group_id() < 8) {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_r_idx + (32)];
      uint r0_4 = shared.m[smem_r_idx + (48)];
      {
        uint const t = min(r0_2, r0_3);
        r0_3 = max(r0_2, r0_3);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_4);
        r0_4 = max(r0_1, r0_4);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_r_idx + (32)] = r0_3;
      shared.m[smem_r_idx + (48)] = r0_4;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (64)];
      uint r1_2 = shared.m[smem_l_idx + (80)];
      uint r1_3 = shared.m[smem_r_idx + (96)];
      uint r1_4 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r1_2, r1_3);
        r1_3 = max(r1_2, r1_3);
        r1_2 = t;
      };
      {
        uint const t = min(r1_1, r1_4);
        r1_4 = max(r1_1, r1_4);
        r1_1 = t;
      };
      {
        uint const t = min(r1_3, r1_4);
        r1_4 = max(r1_3, r1_4);
        r1_3 = t;
      };
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r1_1;
      shared.m[smem_l_idx + (80)] = r1_2;
      shared.m[smem_r_idx + (96)] = r1_3;
      shared.m[smem_r_idx + (112)] = r1_4;
    }
    {
      uint r2_1 = shared.m[smem_l_idx + (128)];
      uint r2_2 = shared.m[smem_l_idx + (144)];
      uint r2_3 = shared.m[smem_r_idx + (160)];
      uint r2_4 = shared.m[smem_r_idx + (176)];
      {
        uint const t = min(r2_2, r2_3);
        r2_3 = max(r2_2, r2_3);
        r2_2 = t;
      };
      {
        uint const t = min(r2_1, r2_4);
        r2_4 = max(r2_1, r2_4);
        r2_1 = t;
      };
      {
        uint const t = min(r2_3, r2_4);
        r2_4 = max(r2_3, r2_4);
        r2_3 = t;
      };
      {
        uint const t = min(r2_1, r2_2);
        r2_2 = max(r2_1, r2_2);
        r2_1 = t;
      };
      shared.m[smem_l_idx + (128)] = r2_1;
      shared.m[smem_l_idx + (144)] = r2_2;
      shared.m[smem_r_idx + (160)] = r2_3;
      shared.m[smem_r_idx + (176)] = r2_4;
    }
    {
      uint r3_1 = shared.m[smem_l_idx + (192)];
      uint r3_2 = shared.m[smem_l_idx + (208)];
      uint r3_3 = shared.m[smem_r_idx + (224)];
      uint r3_4 = shared.m[smem_r_idx + (240)];
      {
        uint const t = min(r3_2, r3_3);
        r3_3 = max(r3_2, r3_3);
        r3_2 = t;
      };
      {
        uint const t = min(r3_1, r3_4);
        r3_4 = max(r3_1, r3_4);
        r3_1 = t;
      };
      {
        uint const t = min(r3_3, r3_4);
        r3_4 = max(r3_3, r3_4);
        r3_3 = t;
      };
      {
        uint const t = min(r3_1, r3_2);
        r3_2 = max(r3_1, r3_2);
        r3_1 = t;
      };
      shared.m[smem_l_idx + (192)] = r3_1;
      shared.m[smem_l_idx + (208)] = r3_2;
      shared.m[smem_r_idx + (224)] = r3_3;
      shared.m[smem_r_idx + (240)] = r3_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  if (get_sub_group_id() < 8) {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_l_idx + (32)];
      uint r0_4 = shared.m[smem_l_idx + (48)];
      uint r0_5 = shared.m[smem_r_idx + (64)];
      uint r0_6 = shared.m[smem_r_idx + (80)];
      uint r0_7 = shared.m[smem_r_idx + (96)];
      uint r0_8 = shared.m[smem_r_idx + (112)];
      {
        uint const t = min(r0_4, r0_5);
        r0_5 = max(r0_4, r0_5);
        r0_4 = t;
      };
      {
        uint const t = min(r0_3, r0_6);
        r0_6 = max(r0_3, r0_6);
        r0_3 = t;
      };
      {
        uint const t = min(r0_2, r0_7);
        r0_7 = max(r0_2, r0_7);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_8);
        r0_8 = max(r0_1, r0_8);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_7);
        r0_7 = max(r0_5, r0_7);
        r0_5 = t;
      };
      {
        uint const t = min(r0_6, r0_8);
        r0_8 = max(r0_6, r0_8);
        r0_6 = t;
      };
      {
        uint const t = min(r0_5, r0_6);
        r0_6 = max(r0_5, r0_6);
        r0_5 = t;
      };
      {
        uint const t = min(r0_7, r0_8);
        r0_8 = max(r0_7, r0_8);
        r0_7 = t;
      };
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
      shared.m[smem_r_idx + (64)] = r0_5;
      shared.m[smem_r_idx + (80)] = r0_6;
      shared.m[smem_r_idx + (96)] = r0_7;
      shared.m[smem_r_idx + (112)] = r0_8;
    }
    {
      uint r1_1 = shared.m[smem_l_idx + (128)];
      uint r1_2 = shared.m[smem_l_idx + (144)];
      uint r1_3 = shared.m[smem_l_idx + (160)];
      uint r1_4 = shared.m[smem_l_idx + (176)];
      uint r1_5 = shared.m[smem_r_idx + (192)];
      uint r1_6 = shared.m[smem_r_idx + (208)];
      uint r1_7 = shared.m[smem_r_idx + (224)];
      uint r1_8 = shared.m[smem_r_idx + (240)];
      {
        uint const t = min(r1_4, r1_5);
        r1_5 = max(r1_4, r1_5);
        r1_4 = t;
      };
      {
        uint const t = min(r1_3, r1_6);
        r1_6 = max(r1_3, r1_6);
        r1_3 = t;
      };
      {
        uint const t = min(r1_2, r1_7);
        r1_7 = max(r1_2, r1_7);
        r1_2 = t;
      };
      {
        uint const t = min(r1_1, r1_8);
        r1_8 = max(r1_1, r1_8);
        r1_1 = t;
      };
      {
        uint const t = min(r1_5, r1_7);
        r1_7 = max(r1_5, r1_7);
        r1_5 = t;
      };
      {
        uint const t = min(r1_6, r1_8);
        r1_8 = max(r1_6, r1_8);
        r1_6 = t;
      };
      {
        uint const t = min(r1_5, r1_6);
        r1_6 = max(r1_5, r1_6);
        r1_5 = t;
      };
      {
        uint const t = min(r1_7, r1_8);
        r1_8 = max(r1_7, r1_8);
        r1_7 = t;
      };
      {
        uint const t = min(r1_1, r1_3);
        r1_3 = max(r1_1, r1_3);
        r1_1 = t;
      };
      {
        uint const t = min(r1_2, r1_4);
        r1_4 = max(r1_2, r1_4);
        r1_2 = t;
      };
      {
        uint const t = min(r1_1, r1_2);
        r1_2 = max(r1_1, r1_2);
        r1_1 = t;
      };
      {
        uint const t = min(r1_3, r1_4);
        r1_4 = max(r1_3, r1_4);
        r1_3 = t;
      };
      shared.m[smem_l_idx + (128)] = r1_1;
      shared.m[smem_l_idx + (144)] = r1_2;
      shared.m[smem_l_idx + (160)] = r1_3;
      shared.m[smem_l_idx + (176)] = r1_4;
      shared.m[smem_r_idx + (192)] = r1_5;
      shared.m[smem_r_idx + (208)] = r1_6;
      shared.m[smem_r_idx + (224)] = r1_7;
      shared.m[smem_r_idx + (240)] = r1_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  shared.m[get_local_id(0) + (16 * (1 << 4) * 0)] = r1;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 1)] = r8;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 2)] = r2;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 3)] = r7;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 4)] = r3;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 5)] = r6;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 6)] = r4;
  shared.m[get_local_id(0) + (16 * (1 << 4) * 7)] = r5;
  barrier(CLK_LOCAL_MEM_FENCE);
  if (get_sub_group_id() < 8) {
    {
      uint r0_1 = shared.m[smem_l_idx + (0)];
      uint r0_2 = shared.m[smem_l_idx + (16)];
      uint r0_3 = shared.m[smem_l_idx + (32)];
      uint r0_4 = shared.m[smem_l_idx + (48)];
      uint r0_5 = shared.m[smem_l_idx + (64)];
      uint r0_6 = shared.m[smem_l_idx + (80)];
      uint r0_7 = shared.m[smem_l_idx + (96)];
      uint r0_8 = shared.m[smem_l_idx + (112)];
      uint r0_9 = shared.m[smem_r_idx + (128)];
      uint r0_10 = shared.m[smem_r_idx + (144)];
      uint r0_11 = shared.m[smem_r_idx + (160)];
      uint r0_12 = shared.m[smem_r_idx + (176)];
      uint r0_13 = shared.m[smem_r_idx + (192)];
      uint r0_14 = shared.m[smem_r_idx + (208)];
      uint r0_15 = shared.m[smem_r_idx + (224)];
      uint r0_16 = shared.m[smem_r_idx + (240)];
      {
        uint const t = min(r0_8, r0_9);
        r0_9 = max(r0_8, r0_9);
        r0_8 = t;
      };
      {
        uint const t = min(r0_7, r0_10);
        r0_10 = max(r0_7, r0_10);
        r0_7 = t;
      };
      {
        uint const t = min(r0_6, r0_11);
        r0_11 = max(r0_6, r0_11);
        r0_6 = t;
      };
      {
        uint const t = min(r0_5, r0_12);
        r0_12 = max(r0_5, r0_12);
        r0_5 = t;
      };
      {
        uint const t = min(r0_4, r0_13);
        r0_13 = max(r0_4, r0_13);
        r0_4 = t;
      };
      {
        uint const t = min(r0_3, r0_14);
        r0_14 = max(r0_3, r0_14);
        r0_3 = t;
      };
      {
        uint const t = min(r0_2, r0_15);
        r0_15 = max(r0_2, r0_15);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_16);
        r0_16 = max(r0_1, r0_16);
        r0_1 = t;
      };
      {
        uint const t = min(r0_9, r0_13);
        r0_13 = max(r0_9, r0_13);
        r0_9 = t;
      };
      {
        uint const t = min(r0_11, r0_15);
        r0_15 = max(r0_11, r0_15);
        r0_11 = t;
      };
      {
        uint const t = min(r0_9, r0_11);
        r0_11 = max(r0_9, r0_11);
        r0_9 = t;
      };
      {
        uint const t = min(r0_13, r0_15);
        r0_15 = max(r0_13, r0_15);
        r0_13 = t;
      };
      {
        uint const t = min(r0_10, r0_14);
        r0_14 = max(r0_10, r0_14);
        r0_10 = t;
      };
      {
        uint const t = min(r0_12, r0_16);
        r0_16 = max(r0_12, r0_16);
        r0_12 = t;
      };
      {
        uint const t = min(r0_10, r0_12);
        r0_12 = max(r0_10, r0_12);
        r0_10 = t;
      };
      {
        uint const t = min(r0_14, r0_16);
        r0_16 = max(r0_14, r0_16);
        r0_14 = t;
      };
      {
        uint const t = min(r0_9, r0_10);
        r0_10 = max(r0_9, r0_10);
        r0_9 = t;
      };
      {
        uint const t = min(r0_11, r0_12);
        r0_12 = max(r0_11, r0_12);
        r0_11 = t;
      };
      {
        uint const t = min(r0_13, r0_14);
        r0_14 = max(r0_13, r0_14);
        r0_13 = t;
      };
      {
        uint const t = min(r0_15, r0_16);
        r0_16 = max(r0_15, r0_16);
        r0_15 = t;
      };
      {
        uint const t = min(r0_1, r0_5);
        r0_5 = max(r0_1, r0_5);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_7);
        r0_7 = max(r0_3, r0_7);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_7);
        r0_7 = max(r0_5, r0_7);
        r0_5 = t;
      };
      {
        uint const t = min(r0_2, r0_6);
        r0_6 = max(r0_2, r0_6);
        r0_2 = t;
      };
      {
        uint const t = min(r0_4, r0_8);
        r0_8 = max(r0_4, r0_8);
        r0_4 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_6, r0_8);
        r0_8 = max(r0_6, r0_8);
        r0_6 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_5, r0_6);
        r0_6 = max(r0_5, r0_6);
        r0_5 = t;
      };
      {
        uint const t = min(r0_7, r0_8);
        r0_8 = max(r0_7, r0_8);
        r0_7 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
      shared.m[smem_l_idx + (64)] = r0_5;
      shared.m[smem_l_idx + (80)] = r0_6;
      shared.m[smem_l_idx + (96)] = r0_7;
      shared.m[smem_l_idx + (112)] = r0_8;
      shared.m[smem_r_idx + (128)] = r0_9;
      shared.m[smem_r_idx + (144)] = r0_10;
      shared.m[smem_r_idx + (160)] = r0_11;
      shared.m[smem_r_idx + (176)] = r0_12;
      shared.m[smem_r_idx + (192)] = r0_13;
      shared.m[smem_r_idx + (208)] = r0_14;
      shared.m[smem_r_idx + (224)] = r0_15;
      shared.m[smem_r_idx + (240)] = r0_16;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  r1 = shared.m[get_local_id(0) + (16 * (1 << 4) * 0)];
  r8 = shared.m[get_local_id(0) + (16 * (1 << 4) * 1)];
  r2 = shared.m[get_local_id(0) + (16 * (1 << 4) * 2)];
  r7 = shared.m[get_local_id(0) + (16 * (1 << 4) * 3)];
  r3 = shared.m[get_local_id(0) + (16 * (1 << 4) * 4)];
  r6 = shared.m[get_local_id(0) + (16 * (1 << 4) * 5)];
  r4 = shared.m[get_local_id(0) + (16 * (1 << 4) * 6)];
  r5 = shared.m[get_local_id(0) + (16 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 1, 1, 1))) void
hs_kernel_bc_0(__global uint* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vout[gmem_idx + (1 << 4) * 0];
  uint r2 = vout[gmem_idx + (1 << 4) * 1];
  uint r3 = vout[gmem_idx + (1 << 4) * 2];
  uint r4 = vout[gmem_idx + (1 << 4) * 3];
  uint r5 = vout[gmem_idx + (1 << 4) * 4];
  uint r6 = vout[gmem_idx + (1 << 4) * 5];
  uint r7 = vout[gmem_idx + (1 << 4) * 6];
  uint r8 = vout[gmem_idx + (1 << 4) * 7];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 2, 1, 1))) void
hs_kernel_bc_1(__global uint* const restrict vout)
{
  __local struct
  {
    uint m[32 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 4) * 2 - 1)) * 8 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 2) + get_sub_group_local_id();
  {
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 0)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 8)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
    }
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 2)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 10)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (64)] = r0_1;
      shared.m[smem_l_idx + (80)] = r0_2;
    }
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 4)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 12)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (128)] = r0_1;
      shared.m[smem_l_idx + (144)] = r0_2;
    }
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 6)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 14)];
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      shared.m[smem_l_idx + (192)] = r0_1;
      shared.m[smem_l_idx + (208)] = r0_2;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  uint r1 = shared.m[get_local_id(0) + (2 * (1 << 4) * 0)];
  uint r2 = shared.m[get_local_id(0) + (2 * (1 << 4) * 1)];
  uint r3 = shared.m[get_local_id(0) + (2 * (1 << 4) * 2)];
  uint r4 = shared.m[get_local_id(0) + (2 * (1 << 4) * 3)];
  uint r5 = shared.m[get_local_id(0) + (2 * (1 << 4) * 4)];
  uint r6 = shared.m[get_local_id(0) + (2 * (1 << 4) * 5)];
  uint r7 = shared.m[get_local_id(0) + (2 * (1 << 4) * 6)];
  uint r8 = shared.m[get_local_id(0) + (2 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 4, 1, 1))) void
hs_kernel_bc_2(__global uint* const restrict vout)
{
  __local struct
  {
    uint m[64 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 4) * 4 - 1)) * 8 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 4) + get_sub_group_local_id();
  {
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 0)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 8)];
      uint r0_3 = vout[gmem_l_idx + ((1 << 4) * 16)];
      uint r0_4 = vout[gmem_l_idx + ((1 << 4) * 24)];
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
    }
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 4)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 12)];
      uint r0_3 = vout[gmem_l_idx + ((1 << 4) * 20)];
      uint r0_4 = vout[gmem_l_idx + ((1 << 4) * 28)];
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      shared.m[smem_l_idx + (256)] = r0_1;
      shared.m[smem_l_idx + (272)] = r0_2;
      shared.m[smem_l_idx + (288)] = r0_3;
      shared.m[smem_l_idx + (304)] = r0_4;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  uint r1 = shared.m[get_local_id(0) + (4 * (1 << 4) * 0)];
  uint r2 = shared.m[get_local_id(0) + (4 * (1 << 4) * 1)];
  uint r3 = shared.m[get_local_id(0) + (4 * (1 << 4) * 2)];
  uint r4 = shared.m[get_local_id(0) + (4 * (1 << 4) * 3)];
  uint r5 = shared.m[get_local_id(0) + (4 * (1 << 4) * 4)];
  uint r6 = shared.m[get_local_id(0) + (4 * (1 << 4) * 5)];
  uint r7 = shared.m[get_local_id(0) + (4 * (1 << 4) * 6)];
  uint r8 = shared.m[get_local_id(0) + (4 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 8, 1, 1))) void
hs_kernel_bc_3(__global uint* const restrict vout)
{
  __local struct
  {
    uint m[128 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 4) * 8 - 1)) * 8 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 8) + get_sub_group_local_id();
  {
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 0)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 8)];
      uint r0_3 = vout[gmem_l_idx + ((1 << 4) * 16)];
      uint r0_4 = vout[gmem_l_idx + ((1 << 4) * 24)];
      uint r0_5 = vout[gmem_l_idx + ((1 << 4) * 32)];
      uint r0_6 = vout[gmem_l_idx + ((1 << 4) * 40)];
      uint r0_7 = vout[gmem_l_idx + ((1 << 4) * 48)];
      uint r0_8 = vout[gmem_l_idx + ((1 << 4) * 56)];
      {
        uint const t = min(r0_1, r0_5);
        r0_5 = max(r0_1, r0_5);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_7);
        r0_7 = max(r0_3, r0_7);
        r0_3 = t;
      };
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_7);
        r0_7 = max(r0_5, r0_7);
        r0_5 = t;
      };
      {
        uint const t = min(r0_2, r0_6);
        r0_6 = max(r0_2, r0_6);
        r0_2 = t;
      };
      {
        uint const t = min(r0_4, r0_8);
        r0_8 = max(r0_4, r0_8);
        r0_4 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_6, r0_8);
        r0_8 = max(r0_6, r0_8);
        r0_6 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_5, r0_6);
        r0_6 = max(r0_5, r0_6);
        r0_5 = t;
      };
      {
        uint const t = min(r0_7, r0_8);
        r0_8 = max(r0_7, r0_8);
        r0_7 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
      shared.m[smem_l_idx + (64)] = r0_5;
      shared.m[smem_l_idx + (80)] = r0_6;
      shared.m[smem_l_idx + (96)] = r0_7;
      shared.m[smem_l_idx + (112)] = r0_8;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  uint r1 = shared.m[get_local_id(0) + (8 * (1 << 4) * 0)];
  uint r2 = shared.m[get_local_id(0) + (8 * (1 << 4) * 1)];
  uint r3 = shared.m[get_local_id(0) + (8 * (1 << 4) * 2)];
  uint r4 = shared.m[get_local_id(0) + (8 * (1 << 4) * 3)];
  uint r5 = shared.m[get_local_id(0) + (8 * (1 << 4) * 4)];
  uint r6 = shared.m[get_local_id(0) + (8 * (1 << 4) * 5)];
  uint r7 = shared.m[get_local_id(0) + (8 * (1 << 4) * 6)];
  uint r8 = shared.m[get_local_id(0) + (8 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4))))
__attribute__((reqd_work_group_size((1 << 4) * 16, 1, 1))) void
hs_kernel_bc_4(__global uint* const restrict vout)
{
  __local struct
  {
    uint m[256 * 8];
  } shared;

  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint const gmem_l_idx =
    (get_global_id(0) & ~((1 << 4) * 16 - 1)) * 8 + get_local_id(0);
  uint const smem_l_idx =
    get_sub_group_id() * ((1 << 4) * 16) + get_sub_group_local_id();
  if (get_sub_group_id() < 8) {
    {
      uint r0_1 = vout[gmem_l_idx + ((1 << 4) * 0)];
      uint r0_2 = vout[gmem_l_idx + ((1 << 4) * 8)];
      uint r0_3 = vout[gmem_l_idx + ((1 << 4) * 16)];
      uint r0_4 = vout[gmem_l_idx + ((1 << 4) * 24)];
      uint r0_5 = vout[gmem_l_idx + ((1 << 4) * 32)];
      uint r0_6 = vout[gmem_l_idx + ((1 << 4) * 40)];
      uint r0_7 = vout[gmem_l_idx + ((1 << 4) * 48)];
      uint r0_8 = vout[gmem_l_idx + ((1 << 4) * 56)];
      uint r0_9 = vout[gmem_l_idx + ((1 << 4) * 64)];
      uint r0_10 = vout[gmem_l_idx + ((1 << 4) * 72)];
      uint r0_11 = vout[gmem_l_idx + ((1 << 4) * 80)];
      uint r0_12 = vout[gmem_l_idx + ((1 << 4) * 88)];
      uint r0_13 = vout[gmem_l_idx + ((1 << 4) * 96)];
      uint r0_14 = vout[gmem_l_idx + ((1 << 4) * 104)];
      uint r0_15 = vout[gmem_l_idx + ((1 << 4) * 112)];
      uint r0_16 = vout[gmem_l_idx + ((1 << 4) * 120)];
      {
        uint const t = min(r0_1, r0_9);
        r0_9 = max(r0_1, r0_9);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_13);
        r0_13 = max(r0_5, r0_13);
        r0_5 = t;
      };
      {
        uint const t = min(r0_1, r0_5);
        r0_5 = max(r0_1, r0_5);
        r0_1 = t;
      };
      {
        uint const t = min(r0_9, r0_13);
        r0_13 = max(r0_9, r0_13);
        r0_9 = t;
      };
      {
        uint const t = min(r0_3, r0_11);
        r0_11 = max(r0_3, r0_11);
        r0_3 = t;
      };
      {
        uint const t = min(r0_7, r0_15);
        r0_15 = max(r0_7, r0_15);
        r0_7 = t;
      };
      {
        uint const t = min(r0_3, r0_7);
        r0_7 = max(r0_3, r0_7);
        r0_3 = t;
      };
      {
        uint const t = min(r0_11, r0_15);
        r0_15 = max(r0_11, r0_15);
        r0_11 = t;
      };
      {
        uint const t = min(r0_1, r0_3);
        r0_3 = max(r0_1, r0_3);
        r0_1 = t;
      };
      {
        uint const t = min(r0_5, r0_7);
        r0_7 = max(r0_5, r0_7);
        r0_5 = t;
      };
      {
        uint const t = min(r0_9, r0_11);
        r0_11 = max(r0_9, r0_11);
        r0_9 = t;
      };
      {
        uint const t = min(r0_13, r0_15);
        r0_15 = max(r0_13, r0_15);
        r0_13 = t;
      };
      {
        uint const t = min(r0_2, r0_10);
        r0_10 = max(r0_2, r0_10);
        r0_2 = t;
      };
      {
        uint const t = min(r0_6, r0_14);
        r0_14 = max(r0_6, r0_14);
        r0_6 = t;
      };
      {
        uint const t = min(r0_2, r0_6);
        r0_6 = max(r0_2, r0_6);
        r0_2 = t;
      };
      {
        uint const t = min(r0_10, r0_14);
        r0_14 = max(r0_10, r0_14);
        r0_10 = t;
      };
      {
        uint const t = min(r0_4, r0_12);
        r0_12 = max(r0_4, r0_12);
        r0_4 = t;
      };
      {
        uint const t = min(r0_8, r0_16);
        r0_16 = max(r0_8, r0_16);
        r0_8 = t;
      };
      {
        uint const t = min(r0_4, r0_8);
        r0_8 = max(r0_4, r0_8);
        r0_4 = t;
      };
      {
        uint const t = min(r0_12, r0_16);
        r0_16 = max(r0_12, r0_16);
        r0_12 = t;
      };
      {
        uint const t = min(r0_2, r0_4);
        r0_4 = max(r0_2, r0_4);
        r0_2 = t;
      };
      {
        uint const t = min(r0_6, r0_8);
        r0_8 = max(r0_6, r0_8);
        r0_6 = t;
      };
      {
        uint const t = min(r0_10, r0_12);
        r0_12 = max(r0_10, r0_12);
        r0_10 = t;
      };
      {
        uint const t = min(r0_14, r0_16);
        r0_16 = max(r0_14, r0_16);
        r0_14 = t;
      };
      {
        uint const t = min(r0_1, r0_2);
        r0_2 = max(r0_1, r0_2);
        r0_1 = t;
      };
      {
        uint const t = min(r0_3, r0_4);
        r0_4 = max(r0_3, r0_4);
        r0_3 = t;
      };
      {
        uint const t = min(r0_5, r0_6);
        r0_6 = max(r0_5, r0_6);
        r0_5 = t;
      };
      {
        uint const t = min(r0_7, r0_8);
        r0_8 = max(r0_7, r0_8);
        r0_7 = t;
      };
      {
        uint const t = min(r0_9, r0_10);
        r0_10 = max(r0_9, r0_10);
        r0_9 = t;
      };
      {
        uint const t = min(r0_11, r0_12);
        r0_12 = max(r0_11, r0_12);
        r0_11 = t;
      };
      {
        uint const t = min(r0_13, r0_14);
        r0_14 = max(r0_13, r0_14);
        r0_13 = t;
      };
      {
        uint const t = min(r0_15, r0_16);
        r0_16 = max(r0_15, r0_16);
        r0_15 = t;
      };
      shared.m[smem_l_idx + (0)] = r0_1;
      shared.m[smem_l_idx + (16)] = r0_2;
      shared.m[smem_l_idx + (32)] = r0_3;
      shared.m[smem_l_idx + (48)] = r0_4;
      shared.m[smem_l_idx + (64)] = r0_5;
      shared.m[smem_l_idx + (80)] = r0_6;
      shared.m[smem_l_idx + (96)] = r0_7;
      shared.m[smem_l_idx + (112)] = r0_8;
      shared.m[smem_l_idx + (128)] = r0_9;
      shared.m[smem_l_idx + (144)] = r0_10;
      shared.m[smem_l_idx + (160)] = r0_11;
      shared.m[smem_l_idx + (176)] = r0_12;
      shared.m[smem_l_idx + (192)] = r0_13;
      shared.m[smem_l_idx + (208)] = r0_14;
      shared.m[smem_l_idx + (224)] = r0_15;
      shared.m[smem_l_idx + (240)] = r0_16;
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  uint r1 = shared.m[get_local_id(0) + (16 * (1 << 4) * 0)];
  uint r2 = shared.m[get_local_id(0) + (16 * (1 << 4) * 1)];
  uint r3 = shared.m[get_local_id(0) + (16 * (1 << 4) * 2)];
  uint r4 = shared.m[get_local_id(0) + (16 * (1 << 4) * 3)];
  uint r5 = shared.m[get_local_id(0) + (16 * (1 << 4) * 4)];
  uint r6 = shared.m[get_local_id(0) + (16 * (1 << 4) * 5)];
  uint r7 = shared.m[get_local_id(0) + (16 * (1 << 4) * 6)];
  uint r8 = shared.m[get_local_id(0) + (16 * (1 << 4) * 7)];
  {
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 8;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 4;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 2;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const half_lane_idx = get_sub_group_local_id() ^ 1;
      int const t_lt = get_sub_group_local_id() < half_lane_idx;
      ;
      {
        uint const ta = intel_sub_group_shuffle(r1, half_lane_idx);
        r1 = ((r1 <= ta) ^ t_lt) ? ta : r1;
      };
      {
        uint const ta = intel_sub_group_shuffle(r2, half_lane_idx);
        r2 = ((r2 <= ta) ^ t_lt) ? ta : r2;
      };
      {
        uint const ta = intel_sub_group_shuffle(r3, half_lane_idx);
        r3 = ((r3 <= ta) ^ t_lt) ? ta : r3;
      };
      {
        uint const ta = intel_sub_group_shuffle(r4, half_lane_idx);
        r4 = ((r4 <= ta) ^ t_lt) ? ta : r4;
      };
      {
        uint const ta = intel_sub_group_shuffle(r5, half_lane_idx);
        r5 = ((r5 <= ta) ^ t_lt) ? ta : r5;
      };
      {
        uint const ta = intel_sub_group_shuffle(r6, half_lane_idx);
        r6 = ((r6 <= ta) ^ t_lt) ? ta : r6;
      };
      {
        uint const ta = intel_sub_group_shuffle(r7, half_lane_idx);
        r7 = ((r7 <= ta) ^ t_lt) ? ta : r7;
      };
      {
        uint const ta = intel_sub_group_shuffle(r8, half_lane_idx);
        r8 = ((r8 <= ta) ^ t_lt) ? ta : r8;
      };
    }
    {
      uint const t = min(r1, r5);
      r5 = max(r1, r5);
      r1 = t;
    };
    {
      uint const t = min(r3, r7);
      r7 = max(r3, r7);
      r3 = t;
    };
    {
      uint const t = min(r1, r3);
      r3 = max(r1, r3);
      r1 = t;
    };
    {
      uint const t = min(r5, r7);
      r7 = max(r5, r7);
      r5 = t;
    };
    {
      uint const t = min(r2, r6);
      r6 = max(r2, r6);
      r2 = t;
    };
    {
      uint const t = min(r4, r8);
      r8 = max(r4, r8);
      r4 = t;
    };
    {
      uint const t = min(r2, r4);
      r4 = max(r2, r4);
      r2 = t;
    };
    {
      uint const t = min(r6, r8);
      r8 = max(r6, r8);
      r6 = t;
    };
    {
      uint const t = min(r1, r2);
      r2 = max(r1, r2);
      r1 = t;
    };
    {
      uint const t = min(r3, r4);
      r4 = max(r3, r4);
      r3 = t;
    };
    {
      uint const t = min(r5, r6);
      r6 = max(r5, r6);
      r5 = t;
    };
    {
      uint const t = min(r7, r8);
      r8 = max(r7, r8);
      r7 = t;
    };
  }
  vout[gmem_idx + (1 << 4) * 0] = r1;
  vout[gmem_idx + (1 << 4) * 1] = r2;
  vout[gmem_idx + (1 << 4) * 2] = r3;
  vout[gmem_idx + (1 << 4) * 3] = r4;
  vout[gmem_idx + (1 << 4) * 4] = r5;
  vout[gmem_idx + (1 << 4) * 5] = r6;
  vout[gmem_idx + (1 << 4) * 6] = r7;
  vout[gmem_idx + (1 << 4) * 7] = r8;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_fm_0_0(__global uint* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 8 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (8 + 1) - span_off - 1;
  uint r1 = vout[span_l + span_stride * 0];
  uint r2 = vout[span_l + span_stride * 1];
  uint r3 = vout[span_l + span_stride * 2];
  uint r4 = vout[span_l + span_stride * 3];
  uint r5 = vout[span_l + span_stride * 4];
  uint r6 = vout[span_l + span_stride * 5];
  uint r7 = vout[span_l + span_stride * 6];
  uint r8 = vout[span_l + span_stride * 7];
  uint r9 = vout[span_r + span_stride * 0];
  {
    uint const t = min(r8, r9);
    r9 = max(r8, r9);
    r8 = t;
  };
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_r + span_stride * 0] = r9;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_fm_0_1(__global uint* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 8 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (8 + 1) - span_off - 1;
  uint r1 = vout[span_l + span_stride * 0];
  uint r2 = vout[span_l + span_stride * 1];
  uint r3 = vout[span_l + span_stride * 2];
  uint r4 = vout[span_l + span_stride * 3];
  uint r5 = vout[span_l + span_stride * 4];
  uint r6 = vout[span_l + span_stride * 5];
  uint r7 = vout[span_l + span_stride * 6];
  uint r8 = vout[span_l + span_stride * 7];
  uint r9 = vout[span_r + span_stride * 0];
  uint r10 = vout[span_r + span_stride * 1];
  {
    uint const t = min(r8, r9);
    r9 = max(r8, r9);
    r8 = t;
  };
  {
    uint const t = min(r7, r10);
    r10 = max(r7, r10);
    r7 = t;
  };
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r9, r10);
    r10 = max(r9, r10);
    r9 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_r + span_stride * 0] = r9;
  vout[span_r + span_stride * 1] = r10;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_fm_0_2(__global uint* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 8 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (8 + 1) - span_off - 1;
  uint r1 = vout[span_l + span_stride * 0];
  uint r2 = vout[span_l + span_stride * 1];
  uint r3 = vout[span_l + span_stride * 2];
  uint r4 = vout[span_l + span_stride * 3];
  uint r5 = vout[span_l + span_stride * 4];
  uint r6 = vout[span_l + span_stride * 5];
  uint r7 = vout[span_l + span_stride * 6];
  uint r8 = vout[span_l + span_stride * 7];
  uint r9 = vout[span_r + span_stride * 0];
  uint r10 = vout[span_r + span_stride * 1];
  uint r11 = vout[span_r + span_stride * 2];
  uint r12 = vout[span_r + span_stride * 3];
  {
    uint const t = min(r8, r9);
    r9 = max(r8, r9);
    r8 = t;
  };
  {
    uint const t = min(r7, r10);
    r10 = max(r7, r10);
    r7 = t;
  };
  {
    uint const t = min(r6, r11);
    r11 = max(r6, r11);
    r6 = t;
  };
  {
    uint const t = min(r5, r12);
    r12 = max(r5, r12);
    r5 = t;
  };
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r9, r11);
    r11 = max(r9, r11);
    r9 = t;
  };
  {
    uint const t = min(r10, r12);
    r12 = max(r10, r12);
    r10 = t;
  };
  {
    uint const t = min(r9, r10);
    r10 = max(r9, r10);
    r9 = t;
  };
  {
    uint const t = min(r11, r12);
    r12 = max(r11, r12);
    r11 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_r + span_stride * 0] = r9;
  vout[span_r + span_stride * 1] = r10;
  vout[span_r + span_stride * 2] = r11;
  vout[span_r + span_stride * 3] = r12;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_fm_0_3(__global uint* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 8 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint const span_r = span_base + span_stride * (8 + 1) - span_off - 1;
  uint r1 = vout[span_l + span_stride * 0];
  uint r2 = vout[span_l + span_stride * 1];
  uint r3 = vout[span_l + span_stride * 2];
  uint r4 = vout[span_l + span_stride * 3];
  uint r5 = vout[span_l + span_stride * 4];
  uint r6 = vout[span_l + span_stride * 5];
  uint r7 = vout[span_l + span_stride * 6];
  uint r8 = vout[span_l + span_stride * 7];
  uint r9 = vout[span_r + span_stride * 0];
  uint r10 = vout[span_r + span_stride * 1];
  uint r11 = vout[span_r + span_stride * 2];
  uint r12 = vout[span_r + span_stride * 3];
  uint r13 = vout[span_r + span_stride * 4];
  uint r14 = vout[span_r + span_stride * 5];
  uint r15 = vout[span_r + span_stride * 6];
  uint r16 = vout[span_r + span_stride * 7];
  {
    uint const t = min(r8, r9);
    r9 = max(r8, r9);
    r8 = t;
  };
  {
    uint const t = min(r7, r10);
    r10 = max(r7, r10);
    r7 = t;
  };
  {
    uint const t = min(r6, r11);
    r11 = max(r6, r11);
    r6 = t;
  };
  {
    uint const t = min(r5, r12);
    r12 = max(r5, r12);
    r5 = t;
  };
  {
    uint const t = min(r4, r13);
    r13 = max(r4, r13);
    r4 = t;
  };
  {
    uint const t = min(r3, r14);
    r14 = max(r3, r14);
    r3 = t;
  };
  {
    uint const t = min(r2, r15);
    r15 = max(r2, r15);
    r2 = t;
  };
  {
    uint const t = min(r1, r16);
    r16 = max(r1, r16);
    r1 = t;
  };
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r9, r13);
    r13 = max(r9, r13);
    r9 = t;
  };
  {
    uint const t = min(r11, r15);
    r15 = max(r11, r15);
    r11 = t;
  };
  {
    uint const t = min(r9, r11);
    r11 = max(r9, r11);
    r9 = t;
  };
  {
    uint const t = min(r13, r15);
    r15 = max(r13, r15);
    r13 = t;
  };
  {
    uint const t = min(r10, r14);
    r14 = max(r10, r14);
    r10 = t;
  };
  {
    uint const t = min(r12, r16);
    r16 = max(r12, r16);
    r12 = t;
  };
  {
    uint const t = min(r10, r12);
    r12 = max(r10, r12);
    r10 = t;
  };
  {
    uint const t = min(r14, r16);
    r16 = max(r14, r16);
    r14 = t;
  };
  {
    uint const t = min(r9, r10);
    r10 = max(r9, r10);
    r9 = t;
  };
  {
    uint const t = min(r11, r12);
    r12 = max(r11, r12);
    r11 = t;
  };
  {
    uint const t = min(r13, r14);
    r14 = max(r13, r14);
    r13 = t;
  };
  {
    uint const t = min(r15, r16);
    r16 = max(r15, r16);
    r15 = t;
  };
  vout[span_l + span_stride * 0] = r1;
  vout[span_l + span_stride * 1] = r2;
  vout[span_l + span_stride * 2] = r3;
  vout[span_l + span_stride * 3] = r4;
  vout[span_l + span_stride * 4] = r5;
  vout[span_l + span_stride * 5] = r6;
  vout[span_l + span_stride * 6] = r7;
  vout[span_l + span_stride * 7] = r8;
  vout[span_r + span_stride * 0] = r9;
  vout[span_r + span_stride * 1] = r10;
  vout[span_r + span_stride * 2] = r11;
  vout[span_r + span_stride * 3] = r12;
  vout[span_r + span_stride * 4] = r13;
  vout[span_r + span_stride * 5] = r14;
  vout[span_r + span_stride * 6] = r15;
  vout[span_r + span_stride * 7] = r16;
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_hm_0(__global uint* const restrict vout)
{
  uint const span_idx = get_global_id(1);
  uint const span_stride = get_global_size(0);
  uint const span_size = span_stride * 8 * 2;
  uint const span_base = span_idx * span_size;
  uint const span_off = get_global_id(0);
  uint const span_l = span_base + span_off;
  uint r1 = vout[span_l + span_stride * 0];
  uint r2 = vout[span_l + span_stride * 1];
  uint r3 = vout[span_l + span_stride * 2];
  uint r4 = vout[span_l + span_stride * 3];
  uint r5 = vout[span_l + span_stride * 4];
  uint r6 = vout[span_l + span_stride * 5];
  uint r7 = vout[span_l + span_stride * 6];
  uint r8 = vout[span_l + span_stride * 7];
  uint r9 = vout[span_l + span_stride * 8];
  uint r10 = vout[span_l + span_stride * 9];
  uint r11 = vout[span_l + span_stride * 10];
  uint r12 = vout[span_l + span_stride * 11];
  uint r13 = vout[span_l + span_stride * 12];
  uint r14 = vout[span_l + span_stride * 13];
  uint r15 = vout[span_l + span_stride * 14];
  uint r16 = vout[span_l + span_stride * 15];
  {
    uint const t = min(r1, r9);
    r9 = max(r1, r9);
    r1 = t;
  };
  {
    uint const t = min(r5, r13);
    r13 = max(r5, r13);
    r5 = t;
  };
  {
    uint const t = min(r1, r5);
    r5 = max(r1, r5);
    r1 = t;
  };
  {
    uint const t = min(r9, r13);
    r13 = max(r9, r13);
    r9 = t;
  };
  {
    uint const t = min(r3, r11);
    r11 = max(r3, r11);
    r3 = t;
  };
  {
    uint const t = min(r7, r15);
    r15 = max(r7, r15);
    r7 = t;
  };
  {
    uint const t = min(r3, r7);
    r7 = max(r3, r7);
    r3 = t;
  };
  {
    uint const t = min(r11, r15);
    r15 = max(r11, r15);
    r11 = t;
  };
  {
    uint const t = min(r1, r3);
    r3 = max(r1, r3);
    r1 = t;
  };
  {
    uint const t = min(r5, r7);
    r7 = max(r5, r7);
    r5 = t;
  };
  {
    uint const t = min(r9, r11);
    r11 = max(r9, r11);
    r9 = t;
  };
  {
    uint const t = min(r13, r15);
    r15 = max(r13, r15);
    r13 = t;
  };
  {
    uint const t = min(r2, r10);
    r10 = max(r2, r10);
    r2 = t;
  };
  {
    uint const t = min(r6, r14);
    r14 = max(r6, r14);
    r6 = t;
  };
  {
    uint const t = min(r2, r6);
    r6 = max(r2, r6);
    r2 = t;
  };
  {
    uint const t = min(r10, r14);
    r14 = max(r10, r14);
    r10 = t;
  };
  {
    uint const t = min(r4, r12);
    r12 = max(r4, r12);
    r4 = t;
  };
  {
    uint const t = min(r8, r16);
    r16 = max(r8, r16);
    r8 = t;
  };
  {
    uint const t = min(r4, r8);
    r8 = max(r4, r8);
    r4 = t;
  };
  {
    uint const t = min(r12, r16);
    r16 = max(r12, r16);
    r12 = t;
  };
  {
    uint const t = min(r2, r4);
    r4 = max(r2, r4);
    r2 = t;
  };
  {
    uint const t = min(r6, r8);
    r8 = max(r6, r8);
    r6 = t;
  };
  {
    uint const t = min(r10, r12);
    r12 = max(r10, r12);
    r10 = t;
  };
  {
    uint const t = min(r14, r16);
    r16 = max(r14, r16);
    r14 = t;
  };
  {
    uint const t = min(r1, r2);
    r2 = max(r1, r2);
    r1 = t;
  };
  {
    uint const t = min(r3, r4);
    r4 = max(r3, r4);
    r3 = t;
  };
  {
    uint const t = min(r5, r6);
    r6 = max(r5, r6);
    r5 = t;
  };
  {
    uint const t = min(r7, r8);
    r8 = max(r7, r8);
    r7 = t;
  };
  {
    uint const t = min(r9, r10);
    r10 = max(r9, r10);
    r9 = t;
  };
  {
    uint const t = min(r11, r12);
    r12 = max(r11, r12);
    r11 = t;
  };
  {
    uint const t = min(r13, r14);
    r14 = max(r13, r14);
    r13 = t;
  };
  {
    uint const t = min(r15, r16);
    r16 = max(r15, r16);
    r15 = t;
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
}

__kernel __attribute__((intel_reqd_sub_group_size((1 << 4)))) void
hs_kernel_transpose(__global uint* const restrict vout)
{
  uint const gmem_idx = (get_global_id(0) & ~((1 << 4) - 1)) * 8 +
                        (get_local_id(0) & ((1 << 4) - 1));
  uint r1 = vout[gmem_idx + (1 << 4) * 0];
  uint r2 = vout[gmem_idx + (1 << 4) * 1];
  uint r3 = vout[gmem_idx + (1 << 4) * 2];
  uint r4 = vout[gmem_idx + (1 << 4) * 3];
  uint r5 = vout[gmem_idx + (1 << 4) * 4];
  uint r6 = vout[gmem_idx + (1 << 4) * 5];
  uint r7 = vout[gmem_idx + (1 << 4) * 6];
  uint r8 = vout[gmem_idx + (1 << 4) * 7];
  bool const is_lo_1 = (get_sub_group_local_id() & (1 << (1 - 1))) == 0;
  bool const is_lo_2 = (get_sub_group_local_id() & (1 << (2 - 1))) == 0;
  bool const is_lo_3 = (get_sub_group_local_id() & (1 << (3 - 1))) == 0;
  bool const is_lo_4 = (get_sub_group_local_id() & (1 << (4 - 1))) == 0;
  uint const s2_1 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r2 : r1, 1 << (1 - 1));
  uint const s2 = is_lo_1 ? s2_1 : r2;
  uint const s1 = is_lo_1 ? r1 : s2_1;
  uint const s4_3 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r4 : r3, 1 << (1 - 1));
  uint const s4 = is_lo_1 ? s4_3 : r4;
  uint const s3 = is_lo_1 ? r3 : s4_3;
  uint const s6_5 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r6 : r5, 1 << (1 - 1));
  uint const s6 = is_lo_1 ? s6_5 : r6;
  uint const s5 = is_lo_1 ? r5 : s6_5;
  uint const s8_7 =
    intel_sub_group_shuffle_xor(is_lo_1 ? r8 : r7, 1 << (1 - 1));
  uint const s8 = is_lo_1 ? s8_7 : r8;
  uint const s7 = is_lo_1 ? r7 : s8_7;
  uint const t3_1 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s3 : s1, 1 << (2 - 1));
  uint const t3 = is_lo_2 ? t3_1 : s3;
  uint const t1 = is_lo_2 ? s1 : t3_1;
  uint const t4_2 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s4 : s2, 1 << (2 - 1));
  uint const t4 = is_lo_2 ? t4_2 : s4;
  uint const t2 = is_lo_2 ? s2 : t4_2;
  uint const t7_5 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s7 : s5, 1 << (2 - 1));
  uint const t7 = is_lo_2 ? t7_5 : s7;
  uint const t5 = is_lo_2 ? s5 : t7_5;
  uint const t8_6 =
    intel_sub_group_shuffle_xor(is_lo_2 ? s8 : s6, 1 << (2 - 1));
  uint const t8 = is_lo_2 ? t8_6 : s8;
  uint const t6 = is_lo_2 ? s6 : t8_6;
  uint const u5_1 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t5 : t1, 1 << (3 - 1));
  uint const u5 = is_lo_3 ? u5_1 : t5;
  uint const u1 = is_lo_3 ? t1 : u5_1;
  uint const u6_2 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t6 : t2, 1 << (3 - 1));
  uint const u6 = is_lo_3 ? u6_2 : t6;
  uint const u2 = is_lo_3 ? t2 : u6_2;
  uint const u7_3 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t7 : t3, 1 << (3 - 1));
  uint const u7 = is_lo_3 ? u7_3 : t7;
  uint const u3 = is_lo_3 ? t3 : u7_3;
  uint const u8_4 =
    intel_sub_group_shuffle_xor(is_lo_3 ? t8 : t4, 1 << (3 - 1));
  uint const u8 = is_lo_3 ? u8_4 : t8;
  uint const u4 = is_lo_3 ? t4 : u8_4;
  uint const v2_1 =
    intel_sub_group_shuffle_xor(is_lo_4 ? u2 : u1, 1 << (4 - 1));
  uint const v2 = is_lo_4 ? v2_1 : u2;
  uint const v1 = is_lo_4 ? u1 : v2_1;
  uint const v4_3 =
    intel_sub_group_shuffle_xor(is_lo_4 ? u4 : u3, 1 << (4 - 1));
  uint const v4 = is_lo_4 ? v4_3 : u4;
  uint const v3 = is_lo_4 ? u3 : v4_3;
  uint const v6_5 =
    intel_sub_group_shuffle_xor(is_lo_4 ? u6 : u5, 1 << (4 - 1));
  uint const v6 = is_lo_4 ? v6_5 : u6;
  uint const v5 = is_lo_4 ? u5 : v6_5;
  uint const v8_7 =
    intel_sub_group_shuffle_xor(is_lo_4 ? u8 : u7, 1 << (4 - 1));
  uint const v8 = is_lo_4 ? v8_7 : u8;
  uint const v7 = is_lo_4 ? u7 : v8_7;
  vout[gmem_idx + ((1 - 1) << 4)] = v1;
  vout[gmem_idx + ((5 - 1) << 4)] = v2;
  vout[gmem_idx + ((2 - 1) << 4)] = v3;
  vout[gmem_idx + ((6 - 1) << 4)] = v4;
  vout[gmem_idx + ((3 - 1) << 4)] = v5;
  vout[gmem_idx + ((7 - 1) << 4)] = v6;
  vout[gmem_idx + ((4 - 1) << 4)] = v7;
  vout[gmem_idx + ((8 - 1) << 4)] = v8;
}
