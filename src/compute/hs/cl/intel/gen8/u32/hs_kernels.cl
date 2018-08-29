//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

// target-specific config
#include "hs_config.h"

// arch/target-specific macros
#include "hs_cl_macros.h"

//
//
//

HS_BS_KERNEL_PROTO(1, 0)
{
  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vin, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vin, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vin, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vin, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vin, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vin, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vin, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vin, 7);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r3, r5);
  HS_CMP_XCHG(r4, r6);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r2, r5);
  HS_CMP_XCHG(r4, r7);
  HS_CMP_XCHG(r2, r3);
  HS_CMP_XCHG(r4, r5);
  HS_CMP_XCHG(r6, r7);
  {
    HS_SLAB_FLIP_PREAMBLE(1);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(3);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(7);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(15);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(4);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BS_KERNEL_PROTO(2, 1)
{
  HS_BLOCK_LOCAL_MEM_DECL(32, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vin, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vin, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vin, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vin, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vin, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vin, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vin, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vin, 7);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r3, r5);
  HS_CMP_XCHG(r4, r6);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r2, r5);
  HS_CMP_XCHG(r4, r7);
  HS_CMP_XCHG(r2, r3);
  HS_CMP_XCHG(r4, r5);
  HS_CMP_XCHG(r6, r7);
  {
    HS_SLAB_FLIP_PREAMBLE(1);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(3);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(7);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(15);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(4);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_BS_MERGE_H_PREAMBLE(2);
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(16);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_R(16) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(80);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(64) = r0_1;
      HS_SLAB_LOCAL_R(80) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(128);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(144);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(128) = r0_1;
      HS_SLAB_LOCAL_R(144) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(192);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(208);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(192) = r0_1;
      HS_SLAB_LOCAL_R(208) = r0_2;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BS_KERNEL_PROTO(4, 2)
{
  HS_BLOCK_LOCAL_MEM_DECL(64, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vin, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vin, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vin, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vin, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vin, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vin, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vin, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vin, 7);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r3, r5);
  HS_CMP_XCHG(r4, r6);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r2, r5);
  HS_CMP_XCHG(r4, r7);
  HS_CMP_XCHG(r2, r3);
  HS_CMP_XCHG(r4, r5);
  HS_CMP_XCHG(r6, r7);
  {
    HS_SLAB_FLIP_PREAMBLE(1);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(3);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(7);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(15);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(4);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_BS_MERGE_H_PREAMBLE(4);
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(16);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_R(16) = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(32) = r1_1;
      HS_SLAB_LOCAL_R(48) = r1_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(256);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(272);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(256) = r0_1;
      HS_SLAB_LOCAL_R(272) = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(288);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_R(304);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(288) = r1_1;
      HS_SLAB_LOCAL_R(304) = r1_2;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_R(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r0_2, r0_3);
      HS_CMP_XCHG(r0_1, r0_4);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_R(32) = r0_3;
      HS_SLAB_LOCAL_R(48) = r0_4;
    }
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(256);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(272);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_R(288);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_R(304);
      HS_CMP_XCHG(r0_2, r0_3);
      HS_CMP_XCHG(r0_1, r0_4);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(256) = r0_1;
      HS_SLAB_LOCAL_L(272) = r0_2;
      HS_SLAB_LOCAL_R(288) = r0_3;
      HS_SLAB_LOCAL_R(304) = r0_4;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BS_KERNEL_PROTO(8, 3)
{
  HS_BLOCK_LOCAL_MEM_DECL(128, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vin, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vin, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vin, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vin, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vin, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vin, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vin, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vin, 7);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r3, r5);
  HS_CMP_XCHG(r4, r6);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r2, r5);
  HS_CMP_XCHG(r4, r7);
  HS_CMP_XCHG(r2, r3);
  HS_CMP_XCHG(r4, r5);
  HS_CMP_XCHG(r6, r7);
  {
    HS_SLAB_FLIP_PREAMBLE(1);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(3);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(7);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(15);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(4);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_BS_MERGE_H_PREAMBLE(8);
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(16);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_R(16) = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(32) = r1_1;
      HS_SLAB_LOCAL_R(48) = r1_2;
    }
    {
      HS_KEY_TYPE r2_1 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r2_2 = HS_SLAB_LOCAL_R(80);
      HS_CMP_XCHG(r2_1, r2_2);
      HS_SLAB_LOCAL_L(64) = r2_1;
      HS_SLAB_LOCAL_R(80) = r2_2;
    }
    {
      HS_KEY_TYPE r3_1 = HS_SLAB_LOCAL_L(96);
      HS_KEY_TYPE r3_2 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r3_1, r3_2);
      HS_SLAB_LOCAL_L(96) = r3_1;
      HS_SLAB_LOCAL_R(112) = r3_2;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_R(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r0_2, r0_3);
      HS_CMP_XCHG(r0_1, r0_4);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_R(32) = r0_3;
      HS_SLAB_LOCAL_R(48) = r0_4;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_L(80);
      HS_KEY_TYPE r1_3 = HS_SLAB_LOCAL_R(96);
      HS_KEY_TYPE r1_4 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r1_2, r1_3);
      HS_CMP_XCHG(r1_1, r1_4);
      HS_CMP_XCHG(r1_3, r1_4);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(64) = r1_1;
      HS_SLAB_LOCAL_L(80) = r1_2;
      HS_SLAB_LOCAL_R(96) = r1_3;
      HS_SLAB_LOCAL_R(112) = r1_4;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_L(48);
      HS_KEY_TYPE r0_5 = HS_SLAB_LOCAL_R(64);
      HS_KEY_TYPE r0_6 = HS_SLAB_LOCAL_R(80);
      HS_KEY_TYPE r0_7 = HS_SLAB_LOCAL_R(96);
      HS_KEY_TYPE r0_8 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r0_4, r0_5);
      HS_CMP_XCHG(r0_3, r0_6);
      HS_CMP_XCHG(r0_2, r0_7);
      HS_CMP_XCHG(r0_1, r0_8);
      HS_CMP_XCHG(r0_5, r0_7);
      HS_CMP_XCHG(r0_6, r0_8);
      HS_CMP_XCHG(r0_5, r0_6);
      HS_CMP_XCHG(r0_7, r0_8);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
      HS_SLAB_LOCAL_R(64) = r0_5;
      HS_SLAB_LOCAL_R(80) = r0_6;
      HS_SLAB_LOCAL_R(96) = r0_7;
      HS_SLAB_LOCAL_R(112) = r0_8;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BS_KERNEL_PROTO(16, 4)
{
  HS_BLOCK_LOCAL_MEM_DECL(256, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vin, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vin, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vin, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vin, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vin, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vin, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vin, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vin, 7);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r3, r5);
  HS_CMP_XCHG(r4, r6);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r2, r5);
  HS_CMP_XCHG(r4, r7);
  HS_CMP_XCHG(r2, r3);
  HS_CMP_XCHG(r4, r5);
  HS_CMP_XCHG(r6, r7);
  {
    HS_SLAB_FLIP_PREAMBLE(1);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(3);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(7);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  {
    HS_SLAB_FLIP_PREAMBLE(15);
    HS_CMP_FLIP(0, r1, r8);
    HS_CMP_FLIP(1, r2, r7);
    HS_CMP_FLIP(2, r3, r6);
    HS_CMP_FLIP(3, r4, r5);
  }
  {
    HS_SLAB_HALF_PREAMBLE(4);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(2);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  {
    HS_SLAB_HALF_PREAMBLE(1);
    HS_CMP_HALF(0, r1);
    HS_CMP_HALF(1, r2);
    HS_CMP_HALF(2, r3);
    HS_CMP_HALF(3, r4);
    HS_CMP_HALF(4, r5);
    HS_CMP_HALF(5, r6);
    HS_CMP_HALF(6, r7);
    HS_CMP_HALF(7, r8);
  }
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_BS_MERGE_H_PREAMBLE(16);
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  if (HS_SUBGROUP_ID() < 8) {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_R(16);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_R(16) = r0_2;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(32) = r1_1;
      HS_SLAB_LOCAL_R(48) = r1_2;
    }
    {
      HS_KEY_TYPE r2_1 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r2_2 = HS_SLAB_LOCAL_R(80);
      HS_CMP_XCHG(r2_1, r2_2);
      HS_SLAB_LOCAL_L(64) = r2_1;
      HS_SLAB_LOCAL_R(80) = r2_2;
    }
    {
      HS_KEY_TYPE r3_1 = HS_SLAB_LOCAL_L(96);
      HS_KEY_TYPE r3_2 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r3_1, r3_2);
      HS_SLAB_LOCAL_L(96) = r3_1;
      HS_SLAB_LOCAL_R(112) = r3_2;
    }
    {
      HS_KEY_TYPE r4_1 = HS_SLAB_LOCAL_L(128);
      HS_KEY_TYPE r4_2 = HS_SLAB_LOCAL_R(144);
      HS_CMP_XCHG(r4_1, r4_2);
      HS_SLAB_LOCAL_L(128) = r4_1;
      HS_SLAB_LOCAL_R(144) = r4_2;
    }
    {
      HS_KEY_TYPE r5_1 = HS_SLAB_LOCAL_L(160);
      HS_KEY_TYPE r5_2 = HS_SLAB_LOCAL_R(176);
      HS_CMP_XCHG(r5_1, r5_2);
      HS_SLAB_LOCAL_L(160) = r5_1;
      HS_SLAB_LOCAL_R(176) = r5_2;
    }
    {
      HS_KEY_TYPE r6_1 = HS_SLAB_LOCAL_L(192);
      HS_KEY_TYPE r6_2 = HS_SLAB_LOCAL_R(208);
      HS_CMP_XCHG(r6_1, r6_2);
      HS_SLAB_LOCAL_L(192) = r6_1;
      HS_SLAB_LOCAL_R(208) = r6_2;
    }
    {
      HS_KEY_TYPE r7_1 = HS_SLAB_LOCAL_L(224);
      HS_KEY_TYPE r7_2 = HS_SLAB_LOCAL_R(240);
      HS_CMP_XCHG(r7_1, r7_2);
      HS_SLAB_LOCAL_L(224) = r7_1;
      HS_SLAB_LOCAL_R(240) = r7_2;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  if (HS_SUBGROUP_ID() < 8) {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_R(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_R(48);
      HS_CMP_XCHG(r0_2, r0_3);
      HS_CMP_XCHG(r0_1, r0_4);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_R(32) = r0_3;
      HS_SLAB_LOCAL_R(48) = r0_4;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_L(80);
      HS_KEY_TYPE r1_3 = HS_SLAB_LOCAL_R(96);
      HS_KEY_TYPE r1_4 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r1_2, r1_3);
      HS_CMP_XCHG(r1_1, r1_4);
      HS_CMP_XCHG(r1_3, r1_4);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_SLAB_LOCAL_L(64) = r1_1;
      HS_SLAB_LOCAL_L(80) = r1_2;
      HS_SLAB_LOCAL_R(96) = r1_3;
      HS_SLAB_LOCAL_R(112) = r1_4;
    }
    {
      HS_KEY_TYPE r2_1 = HS_SLAB_LOCAL_L(128);
      HS_KEY_TYPE r2_2 = HS_SLAB_LOCAL_L(144);
      HS_KEY_TYPE r2_3 = HS_SLAB_LOCAL_R(160);
      HS_KEY_TYPE r2_4 = HS_SLAB_LOCAL_R(176);
      HS_CMP_XCHG(r2_2, r2_3);
      HS_CMP_XCHG(r2_1, r2_4);
      HS_CMP_XCHG(r2_3, r2_4);
      HS_CMP_XCHG(r2_1, r2_2);
      HS_SLAB_LOCAL_L(128) = r2_1;
      HS_SLAB_LOCAL_L(144) = r2_2;
      HS_SLAB_LOCAL_R(160) = r2_3;
      HS_SLAB_LOCAL_R(176) = r2_4;
    }
    {
      HS_KEY_TYPE r3_1 = HS_SLAB_LOCAL_L(192);
      HS_KEY_TYPE r3_2 = HS_SLAB_LOCAL_L(208);
      HS_KEY_TYPE r3_3 = HS_SLAB_LOCAL_R(224);
      HS_KEY_TYPE r3_4 = HS_SLAB_LOCAL_R(240);
      HS_CMP_XCHG(r3_2, r3_3);
      HS_CMP_XCHG(r3_1, r3_4);
      HS_CMP_XCHG(r3_3, r3_4);
      HS_CMP_XCHG(r3_1, r3_2);
      HS_SLAB_LOCAL_L(192) = r3_1;
      HS_SLAB_LOCAL_L(208) = r3_2;
      HS_SLAB_LOCAL_R(224) = r3_3;
      HS_SLAB_LOCAL_R(240) = r3_4;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  if (HS_SUBGROUP_ID() < 8) {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_L(48);
      HS_KEY_TYPE r0_5 = HS_SLAB_LOCAL_R(64);
      HS_KEY_TYPE r0_6 = HS_SLAB_LOCAL_R(80);
      HS_KEY_TYPE r0_7 = HS_SLAB_LOCAL_R(96);
      HS_KEY_TYPE r0_8 = HS_SLAB_LOCAL_R(112);
      HS_CMP_XCHG(r0_4, r0_5);
      HS_CMP_XCHG(r0_3, r0_6);
      HS_CMP_XCHG(r0_2, r0_7);
      HS_CMP_XCHG(r0_1, r0_8);
      HS_CMP_XCHG(r0_5, r0_7);
      HS_CMP_XCHG(r0_6, r0_8);
      HS_CMP_XCHG(r0_5, r0_6);
      HS_CMP_XCHG(r0_7, r0_8);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
      HS_SLAB_LOCAL_R(64) = r0_5;
      HS_SLAB_LOCAL_R(80) = r0_6;
      HS_SLAB_LOCAL_R(96) = r0_7;
      HS_SLAB_LOCAL_R(112) = r0_8;
    }
    {
      HS_KEY_TYPE r1_1 = HS_SLAB_LOCAL_L(128);
      HS_KEY_TYPE r1_2 = HS_SLAB_LOCAL_L(144);
      HS_KEY_TYPE r1_3 = HS_SLAB_LOCAL_L(160);
      HS_KEY_TYPE r1_4 = HS_SLAB_LOCAL_L(176);
      HS_KEY_TYPE r1_5 = HS_SLAB_LOCAL_R(192);
      HS_KEY_TYPE r1_6 = HS_SLAB_LOCAL_R(208);
      HS_KEY_TYPE r1_7 = HS_SLAB_LOCAL_R(224);
      HS_KEY_TYPE r1_8 = HS_SLAB_LOCAL_R(240);
      HS_CMP_XCHG(r1_4, r1_5);
      HS_CMP_XCHG(r1_3, r1_6);
      HS_CMP_XCHG(r1_2, r1_7);
      HS_CMP_XCHG(r1_1, r1_8);
      HS_CMP_XCHG(r1_5, r1_7);
      HS_CMP_XCHG(r1_6, r1_8);
      HS_CMP_XCHG(r1_5, r1_6);
      HS_CMP_XCHG(r1_7, r1_8);
      HS_CMP_XCHG(r1_1, r1_3);
      HS_CMP_XCHG(r1_2, r1_4);
      HS_CMP_XCHG(r1_1, r1_2);
      HS_CMP_XCHG(r1_3, r1_4);
      HS_SLAB_LOCAL_L(128) = r1_1;
      HS_SLAB_LOCAL_L(144) = r1_2;
      HS_SLAB_LOCAL_L(160) = r1_3;
      HS_SLAB_LOCAL_L(176) = r1_4;
      HS_SLAB_LOCAL_R(192) = r1_5;
      HS_SLAB_LOCAL_R(208) = r1_6;
      HS_SLAB_LOCAL_R(224) = r1_7;
      HS_SLAB_LOCAL_R(240) = r1_8;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0) = r1;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1) = r8;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2) = r2;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3) = r7;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4) = r3;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5) = r6;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6) = r4;
  HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7) = r5;
  HS_BLOCK_BARRIER();
  if (HS_SUBGROUP_ID() < 8) {
    {
      HS_KEY_TYPE r0_1 = HS_SLAB_LOCAL_L(0);
      HS_KEY_TYPE r0_2 = HS_SLAB_LOCAL_L(16);
      HS_KEY_TYPE r0_3 = HS_SLAB_LOCAL_L(32);
      HS_KEY_TYPE r0_4 = HS_SLAB_LOCAL_L(48);
      HS_KEY_TYPE r0_5 = HS_SLAB_LOCAL_L(64);
      HS_KEY_TYPE r0_6 = HS_SLAB_LOCAL_L(80);
      HS_KEY_TYPE r0_7 = HS_SLAB_LOCAL_L(96);
      HS_KEY_TYPE r0_8 = HS_SLAB_LOCAL_L(112);
      HS_KEY_TYPE r0_9 = HS_SLAB_LOCAL_R(128);
      HS_KEY_TYPE r0_10 = HS_SLAB_LOCAL_R(144);
      HS_KEY_TYPE r0_11 = HS_SLAB_LOCAL_R(160);
      HS_KEY_TYPE r0_12 = HS_SLAB_LOCAL_R(176);
      HS_KEY_TYPE r0_13 = HS_SLAB_LOCAL_R(192);
      HS_KEY_TYPE r0_14 = HS_SLAB_LOCAL_R(208);
      HS_KEY_TYPE r0_15 = HS_SLAB_LOCAL_R(224);
      HS_KEY_TYPE r0_16 = HS_SLAB_LOCAL_R(240);
      HS_CMP_XCHG(r0_8, r0_9);
      HS_CMP_XCHG(r0_7, r0_10);
      HS_CMP_XCHG(r0_6, r0_11);
      HS_CMP_XCHG(r0_5, r0_12);
      HS_CMP_XCHG(r0_4, r0_13);
      HS_CMP_XCHG(r0_3, r0_14);
      HS_CMP_XCHG(r0_2, r0_15);
      HS_CMP_XCHG(r0_1, r0_16);
      HS_CMP_XCHG(r0_9, r0_13);
      HS_CMP_XCHG(r0_11, r0_15);
      HS_CMP_XCHG(r0_9, r0_11);
      HS_CMP_XCHG(r0_13, r0_15);
      HS_CMP_XCHG(r0_10, r0_14);
      HS_CMP_XCHG(r0_12, r0_16);
      HS_CMP_XCHG(r0_10, r0_12);
      HS_CMP_XCHG(r0_14, r0_16);
      HS_CMP_XCHG(r0_9, r0_10);
      HS_CMP_XCHG(r0_11, r0_12);
      HS_CMP_XCHG(r0_13, r0_14);
      HS_CMP_XCHG(r0_15, r0_16);
      HS_CMP_XCHG(r0_1, r0_5);
      HS_CMP_XCHG(r0_3, r0_7);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_5, r0_7);
      HS_CMP_XCHG(r0_2, r0_6);
      HS_CMP_XCHG(r0_4, r0_8);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_6, r0_8);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_5, r0_6);
      HS_CMP_XCHG(r0_7, r0_8);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
      HS_SLAB_LOCAL_L(64) = r0_5;
      HS_SLAB_LOCAL_L(80) = r0_6;
      HS_SLAB_LOCAL_L(96) = r0_7;
      HS_SLAB_LOCAL_L(112) = r0_8;
      HS_SLAB_LOCAL_R(128) = r0_9;
      HS_SLAB_LOCAL_R(144) = r0_10;
      HS_SLAB_LOCAL_R(160) = r0_11;
      HS_SLAB_LOCAL_R(176) = r0_12;
      HS_SLAB_LOCAL_R(192) = r0_13;
      HS_SLAB_LOCAL_R(208) = r0_14;
      HS_SLAB_LOCAL_R(224) = r0_15;
      HS_SLAB_LOCAL_R(240) = r0_16;
    }
  }
  HS_BLOCK_BARRIER();
  r1 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0);
  r8 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1);
  r2 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2);
  r7 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3);
  r3 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4);
  r6 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5);
  r4 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6);
  r5 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BC_KERNEL_PROTO(1, 0)
{
  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vout, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vout, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vout, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vout, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vout, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vout, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vout, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vout, 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BC_KERNEL_PROTO(2, 1)
{
  HS_BLOCK_LOCAL_MEM_DECL(32, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_BC_MERGE_H_PREAMBLE(2);
  {
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(0);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(8);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(2);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(10);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(64) = r0_1;
      HS_SLAB_LOCAL_L(80) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(4);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(12);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(128) = r0_1;
      HS_SLAB_LOCAL_L(144) = r0_2;
    }
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(6);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(14);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_SLAB_LOCAL_L(192) = r0_1;
      HS_SLAB_LOCAL_L(208) = r0_2;
    }
  }
  HS_BLOCK_BARRIER();
  HS_KEY_TYPE r1 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 0);
  HS_KEY_TYPE r2 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 1);
  HS_KEY_TYPE r3 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 2);
  HS_KEY_TYPE r4 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 3);
  HS_KEY_TYPE r5 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 4);
  HS_KEY_TYPE r6 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 5);
  HS_KEY_TYPE r7 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 6);
  HS_KEY_TYPE r8 = HS_BX_LOCAL_V(2 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BC_KERNEL_PROTO(4, 2)
{
  HS_BLOCK_LOCAL_MEM_DECL(64, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_BC_MERGE_H_PREAMBLE(4);
  {
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(0);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(8);
      HS_KEY_TYPE r0_3 = HS_BC_GLOBAL_LOAD_L(16);
      HS_KEY_TYPE r0_4 = HS_BC_GLOBAL_LOAD_L(24);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
    }
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(4);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(12);
      HS_KEY_TYPE r0_3 = HS_BC_GLOBAL_LOAD_L(20);
      HS_KEY_TYPE r0_4 = HS_BC_GLOBAL_LOAD_L(28);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_SLAB_LOCAL_L(256) = r0_1;
      HS_SLAB_LOCAL_L(272) = r0_2;
      HS_SLAB_LOCAL_L(288) = r0_3;
      HS_SLAB_LOCAL_L(304) = r0_4;
    }
  }
  HS_BLOCK_BARRIER();
  HS_KEY_TYPE r1 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 0);
  HS_KEY_TYPE r2 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 1);
  HS_KEY_TYPE r3 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 2);
  HS_KEY_TYPE r4 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 3);
  HS_KEY_TYPE r5 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 4);
  HS_KEY_TYPE r6 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 5);
  HS_KEY_TYPE r7 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 6);
  HS_KEY_TYPE r8 = HS_BX_LOCAL_V(4 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BC_KERNEL_PROTO(8, 3)
{
  HS_BLOCK_LOCAL_MEM_DECL(128, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_BC_MERGE_H_PREAMBLE(8);
  {
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(0);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(8);
      HS_KEY_TYPE r0_3 = HS_BC_GLOBAL_LOAD_L(16);
      HS_KEY_TYPE r0_4 = HS_BC_GLOBAL_LOAD_L(24);
      HS_KEY_TYPE r0_5 = HS_BC_GLOBAL_LOAD_L(32);
      HS_KEY_TYPE r0_6 = HS_BC_GLOBAL_LOAD_L(40);
      HS_KEY_TYPE r0_7 = HS_BC_GLOBAL_LOAD_L(48);
      HS_KEY_TYPE r0_8 = HS_BC_GLOBAL_LOAD_L(56);
      HS_CMP_XCHG(r0_1, r0_5);
      HS_CMP_XCHG(r0_3, r0_7);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_5, r0_7);
      HS_CMP_XCHG(r0_2, r0_6);
      HS_CMP_XCHG(r0_4, r0_8);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_6, r0_8);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_5, r0_6);
      HS_CMP_XCHG(r0_7, r0_8);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
      HS_SLAB_LOCAL_L(64) = r0_5;
      HS_SLAB_LOCAL_L(80) = r0_6;
      HS_SLAB_LOCAL_L(96) = r0_7;
      HS_SLAB_LOCAL_L(112) = r0_8;
    }
  }
  HS_BLOCK_BARRIER();
  HS_KEY_TYPE r1 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 0);
  HS_KEY_TYPE r2 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 1);
  HS_KEY_TYPE r3 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 2);
  HS_KEY_TYPE r4 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 3);
  HS_KEY_TYPE r5 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 4);
  HS_KEY_TYPE r6 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 5);
  HS_KEY_TYPE r7 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 6);
  HS_KEY_TYPE r8 = HS_BX_LOCAL_V(8 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_BC_KERNEL_PROTO(16, 4)
{
  HS_BLOCK_LOCAL_MEM_DECL(256, 8);

  HS_SLAB_GLOBAL_PREAMBLE();
  HS_BC_MERGE_H_PREAMBLE(16);
  if (HS_SUBGROUP_ID() < 8) {
    {
      HS_KEY_TYPE r0_1 = HS_BC_GLOBAL_LOAD_L(0);
      HS_KEY_TYPE r0_2 = HS_BC_GLOBAL_LOAD_L(8);
      HS_KEY_TYPE r0_3 = HS_BC_GLOBAL_LOAD_L(16);
      HS_KEY_TYPE r0_4 = HS_BC_GLOBAL_LOAD_L(24);
      HS_KEY_TYPE r0_5 = HS_BC_GLOBAL_LOAD_L(32);
      HS_KEY_TYPE r0_6 = HS_BC_GLOBAL_LOAD_L(40);
      HS_KEY_TYPE r0_7 = HS_BC_GLOBAL_LOAD_L(48);
      HS_KEY_TYPE r0_8 = HS_BC_GLOBAL_LOAD_L(56);
      HS_KEY_TYPE r0_9 = HS_BC_GLOBAL_LOAD_L(64);
      HS_KEY_TYPE r0_10 = HS_BC_GLOBAL_LOAD_L(72);
      HS_KEY_TYPE r0_11 = HS_BC_GLOBAL_LOAD_L(80);
      HS_KEY_TYPE r0_12 = HS_BC_GLOBAL_LOAD_L(88);
      HS_KEY_TYPE r0_13 = HS_BC_GLOBAL_LOAD_L(96);
      HS_KEY_TYPE r0_14 = HS_BC_GLOBAL_LOAD_L(104);
      HS_KEY_TYPE r0_15 = HS_BC_GLOBAL_LOAD_L(112);
      HS_KEY_TYPE r0_16 = HS_BC_GLOBAL_LOAD_L(120);
      HS_CMP_XCHG(r0_1, r0_9);
      HS_CMP_XCHG(r0_5, r0_13);
      HS_CMP_XCHG(r0_1, r0_5);
      HS_CMP_XCHG(r0_9, r0_13);
      HS_CMP_XCHG(r0_3, r0_11);
      HS_CMP_XCHG(r0_7, r0_15);
      HS_CMP_XCHG(r0_3, r0_7);
      HS_CMP_XCHG(r0_11, r0_15);
      HS_CMP_XCHG(r0_1, r0_3);
      HS_CMP_XCHG(r0_5, r0_7);
      HS_CMP_XCHG(r0_9, r0_11);
      HS_CMP_XCHG(r0_13, r0_15);
      HS_CMP_XCHG(r0_2, r0_10);
      HS_CMP_XCHG(r0_6, r0_14);
      HS_CMP_XCHG(r0_2, r0_6);
      HS_CMP_XCHG(r0_10, r0_14);
      HS_CMP_XCHG(r0_4, r0_12);
      HS_CMP_XCHG(r0_8, r0_16);
      HS_CMP_XCHG(r0_4, r0_8);
      HS_CMP_XCHG(r0_12, r0_16);
      HS_CMP_XCHG(r0_2, r0_4);
      HS_CMP_XCHG(r0_6, r0_8);
      HS_CMP_XCHG(r0_10, r0_12);
      HS_CMP_XCHG(r0_14, r0_16);
      HS_CMP_XCHG(r0_1, r0_2);
      HS_CMP_XCHG(r0_3, r0_4);
      HS_CMP_XCHG(r0_5, r0_6);
      HS_CMP_XCHG(r0_7, r0_8);
      HS_CMP_XCHG(r0_9, r0_10);
      HS_CMP_XCHG(r0_11, r0_12);
      HS_CMP_XCHG(r0_13, r0_14);
      HS_CMP_XCHG(r0_15, r0_16);
      HS_SLAB_LOCAL_L(0) = r0_1;
      HS_SLAB_LOCAL_L(16) = r0_2;
      HS_SLAB_LOCAL_L(32) = r0_3;
      HS_SLAB_LOCAL_L(48) = r0_4;
      HS_SLAB_LOCAL_L(64) = r0_5;
      HS_SLAB_LOCAL_L(80) = r0_6;
      HS_SLAB_LOCAL_L(96) = r0_7;
      HS_SLAB_LOCAL_L(112) = r0_8;
      HS_SLAB_LOCAL_L(128) = r0_9;
      HS_SLAB_LOCAL_L(144) = r0_10;
      HS_SLAB_LOCAL_L(160) = r0_11;
      HS_SLAB_LOCAL_L(176) = r0_12;
      HS_SLAB_LOCAL_L(192) = r0_13;
      HS_SLAB_LOCAL_L(208) = r0_14;
      HS_SLAB_LOCAL_L(224) = r0_15;
      HS_SLAB_LOCAL_L(240) = r0_16;
    }
  }
  HS_BLOCK_BARRIER();
  HS_KEY_TYPE r1 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 0);
  HS_KEY_TYPE r2 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 1);
  HS_KEY_TYPE r3 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 2);
  HS_KEY_TYPE r4 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 3);
  HS_KEY_TYPE r5 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 4);
  HS_KEY_TYPE r6 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 5);
  HS_KEY_TYPE r7 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 6);
  HS_KEY_TYPE r8 = HS_BX_LOCAL_V(16 * HS_SLAB_THREADS * 7);
  {
    {
      HS_SLAB_HALF_PREAMBLE(8);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(4);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(2);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    {
      HS_SLAB_HALF_PREAMBLE(1);
      HS_CMP_HALF(0, r1);
      HS_CMP_HALF(1, r2);
      HS_CMP_HALF(2, r3);
      HS_CMP_HALF(3, r4);
      HS_CMP_HALF(4, r5);
      HS_CMP_HALF(5, r6);
      HS_CMP_HALF(6, r7);
      HS_CMP_HALF(7, r8);
    }
    HS_CMP_XCHG(r1, r5);
    HS_CMP_XCHG(r3, r7);
    HS_CMP_XCHG(r1, r3);
    HS_CMP_XCHG(r5, r7);
    HS_CMP_XCHG(r2, r6);
    HS_CMP_XCHG(r4, r8);
    HS_CMP_XCHG(r2, r4);
    HS_CMP_XCHG(r6, r8);
    HS_CMP_XCHG(r1, r2);
    HS_CMP_XCHG(r3, r4);
    HS_CMP_XCHG(r5, r6);
    HS_CMP_XCHG(r7, r8);
  }
  HS_SLAB_GLOBAL_STORE(0, r1);
  HS_SLAB_GLOBAL_STORE(1, r2);
  HS_SLAB_GLOBAL_STORE(2, r3);
  HS_SLAB_GLOBAL_STORE(3, r4);
  HS_SLAB_GLOBAL_STORE(4, r5);
  HS_SLAB_GLOBAL_STORE(5, r6);
  HS_SLAB_GLOBAL_STORE(6, r7);
  HS_SLAB_GLOBAL_STORE(7, r8);
}

HS_FM_KERNEL_PROTO(0, 0)
{
  HS_FM_PREAMBLE(8);
  HS_KEY_TYPE r1 = HS_XM_GLOBAL_LOAD_L(0);
  HS_KEY_TYPE r2 = HS_XM_GLOBAL_LOAD_L(1);
  HS_KEY_TYPE r3 = HS_XM_GLOBAL_LOAD_L(2);
  HS_KEY_TYPE r4 = HS_XM_GLOBAL_LOAD_L(3);
  HS_KEY_TYPE r5 = HS_XM_GLOBAL_LOAD_L(4);
  HS_KEY_TYPE r6 = HS_XM_GLOBAL_LOAD_L(5);
  HS_KEY_TYPE r7 = HS_XM_GLOBAL_LOAD_L(6);
  HS_KEY_TYPE r8 = HS_XM_GLOBAL_LOAD_L(7);
  HS_KEY_TYPE r9 = HS_FM_GLOBAL_LOAD_R(0);
  HS_CMP_XCHG(r8, r9);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_XM_GLOBAL_STORE_L(0, r1);
  HS_XM_GLOBAL_STORE_L(1, r2);
  HS_XM_GLOBAL_STORE_L(2, r3);
  HS_XM_GLOBAL_STORE_L(3, r4);
  HS_XM_GLOBAL_STORE_L(4, r5);
  HS_XM_GLOBAL_STORE_L(5, r6);
  HS_XM_GLOBAL_STORE_L(6, r7);
  HS_XM_GLOBAL_STORE_L(7, r8);
  HS_FM_GLOBAL_STORE_R(0, r9);
}

HS_FM_KERNEL_PROTO(0, 1)
{
  HS_FM_PREAMBLE(8);
  HS_KEY_TYPE r1 = HS_XM_GLOBAL_LOAD_L(0);
  HS_KEY_TYPE r2 = HS_XM_GLOBAL_LOAD_L(1);
  HS_KEY_TYPE r3 = HS_XM_GLOBAL_LOAD_L(2);
  HS_KEY_TYPE r4 = HS_XM_GLOBAL_LOAD_L(3);
  HS_KEY_TYPE r5 = HS_XM_GLOBAL_LOAD_L(4);
  HS_KEY_TYPE r6 = HS_XM_GLOBAL_LOAD_L(5);
  HS_KEY_TYPE r7 = HS_XM_GLOBAL_LOAD_L(6);
  HS_KEY_TYPE r8 = HS_XM_GLOBAL_LOAD_L(7);
  HS_KEY_TYPE r9 = HS_FM_GLOBAL_LOAD_R(0);
  HS_KEY_TYPE r10 = HS_FM_GLOBAL_LOAD_R(1);
  HS_CMP_XCHG(r8, r9);
  HS_CMP_XCHG(r7, r10);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r9, r10);
  HS_XM_GLOBAL_STORE_L(0, r1);
  HS_XM_GLOBAL_STORE_L(1, r2);
  HS_XM_GLOBAL_STORE_L(2, r3);
  HS_XM_GLOBAL_STORE_L(3, r4);
  HS_XM_GLOBAL_STORE_L(4, r5);
  HS_XM_GLOBAL_STORE_L(5, r6);
  HS_XM_GLOBAL_STORE_L(6, r7);
  HS_XM_GLOBAL_STORE_L(7, r8);
  HS_FM_GLOBAL_STORE_R(0, r9);
  HS_FM_GLOBAL_STORE_R(1, r10);
}

HS_FM_KERNEL_PROTO(0, 2)
{
  HS_FM_PREAMBLE(8);
  HS_KEY_TYPE r1 = HS_XM_GLOBAL_LOAD_L(0);
  HS_KEY_TYPE r2 = HS_XM_GLOBAL_LOAD_L(1);
  HS_KEY_TYPE r3 = HS_XM_GLOBAL_LOAD_L(2);
  HS_KEY_TYPE r4 = HS_XM_GLOBAL_LOAD_L(3);
  HS_KEY_TYPE r5 = HS_XM_GLOBAL_LOAD_L(4);
  HS_KEY_TYPE r6 = HS_XM_GLOBAL_LOAD_L(5);
  HS_KEY_TYPE r7 = HS_XM_GLOBAL_LOAD_L(6);
  HS_KEY_TYPE r8 = HS_XM_GLOBAL_LOAD_L(7);
  HS_KEY_TYPE r9 = HS_FM_GLOBAL_LOAD_R(0);
  HS_KEY_TYPE r10 = HS_FM_GLOBAL_LOAD_R(1);
  HS_KEY_TYPE r11 = HS_FM_GLOBAL_LOAD_R(2);
  HS_KEY_TYPE r12 = HS_FM_GLOBAL_LOAD_R(3);
  HS_CMP_XCHG(r8, r9);
  HS_CMP_XCHG(r7, r10);
  HS_CMP_XCHG(r6, r11);
  HS_CMP_XCHG(r5, r12);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r9, r11);
  HS_CMP_XCHG(r10, r12);
  HS_CMP_XCHG(r9, r10);
  HS_CMP_XCHG(r11, r12);
  HS_XM_GLOBAL_STORE_L(0, r1);
  HS_XM_GLOBAL_STORE_L(1, r2);
  HS_XM_GLOBAL_STORE_L(2, r3);
  HS_XM_GLOBAL_STORE_L(3, r4);
  HS_XM_GLOBAL_STORE_L(4, r5);
  HS_XM_GLOBAL_STORE_L(5, r6);
  HS_XM_GLOBAL_STORE_L(6, r7);
  HS_XM_GLOBAL_STORE_L(7, r8);
  HS_FM_GLOBAL_STORE_R(0, r9);
  HS_FM_GLOBAL_STORE_R(1, r10);
  HS_FM_GLOBAL_STORE_R(2, r11);
  HS_FM_GLOBAL_STORE_R(3, r12);
}

HS_FM_KERNEL_PROTO(0, 3)
{
  HS_FM_PREAMBLE(8);
  HS_KEY_TYPE r1 = HS_XM_GLOBAL_LOAD_L(0);
  HS_KEY_TYPE r2 = HS_XM_GLOBAL_LOAD_L(1);
  HS_KEY_TYPE r3 = HS_XM_GLOBAL_LOAD_L(2);
  HS_KEY_TYPE r4 = HS_XM_GLOBAL_LOAD_L(3);
  HS_KEY_TYPE r5 = HS_XM_GLOBAL_LOAD_L(4);
  HS_KEY_TYPE r6 = HS_XM_GLOBAL_LOAD_L(5);
  HS_KEY_TYPE r7 = HS_XM_GLOBAL_LOAD_L(6);
  HS_KEY_TYPE r8 = HS_XM_GLOBAL_LOAD_L(7);
  HS_KEY_TYPE r9 = HS_FM_GLOBAL_LOAD_R(0);
  HS_KEY_TYPE r10 = HS_FM_GLOBAL_LOAD_R(1);
  HS_KEY_TYPE r11 = HS_FM_GLOBAL_LOAD_R(2);
  HS_KEY_TYPE r12 = HS_FM_GLOBAL_LOAD_R(3);
  HS_KEY_TYPE r13 = HS_FM_GLOBAL_LOAD_R(4);
  HS_KEY_TYPE r14 = HS_FM_GLOBAL_LOAD_R(5);
  HS_KEY_TYPE r15 = HS_FM_GLOBAL_LOAD_R(6);
  HS_KEY_TYPE r16 = HS_FM_GLOBAL_LOAD_R(7);
  HS_CMP_XCHG(r8, r9);
  HS_CMP_XCHG(r7, r10);
  HS_CMP_XCHG(r6, r11);
  HS_CMP_XCHG(r5, r12);
  HS_CMP_XCHG(r4, r13);
  HS_CMP_XCHG(r3, r14);
  HS_CMP_XCHG(r2, r15);
  HS_CMP_XCHG(r1, r16);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r9, r13);
  HS_CMP_XCHG(r11, r15);
  HS_CMP_XCHG(r9, r11);
  HS_CMP_XCHG(r13, r15);
  HS_CMP_XCHG(r10, r14);
  HS_CMP_XCHG(r12, r16);
  HS_CMP_XCHG(r10, r12);
  HS_CMP_XCHG(r14, r16);
  HS_CMP_XCHG(r9, r10);
  HS_CMP_XCHG(r11, r12);
  HS_CMP_XCHG(r13, r14);
  HS_CMP_XCHG(r15, r16);
  HS_XM_GLOBAL_STORE_L(0, r1);
  HS_XM_GLOBAL_STORE_L(1, r2);
  HS_XM_GLOBAL_STORE_L(2, r3);
  HS_XM_GLOBAL_STORE_L(3, r4);
  HS_XM_GLOBAL_STORE_L(4, r5);
  HS_XM_GLOBAL_STORE_L(5, r6);
  HS_XM_GLOBAL_STORE_L(6, r7);
  HS_XM_GLOBAL_STORE_L(7, r8);
  HS_FM_GLOBAL_STORE_R(0, r9);
  HS_FM_GLOBAL_STORE_R(1, r10);
  HS_FM_GLOBAL_STORE_R(2, r11);
  HS_FM_GLOBAL_STORE_R(3, r12);
  HS_FM_GLOBAL_STORE_R(4, r13);
  HS_FM_GLOBAL_STORE_R(5, r14);
  HS_FM_GLOBAL_STORE_R(6, r15);
  HS_FM_GLOBAL_STORE_R(7, r16);
}

HS_HM_KERNEL_PROTO(0)
{
  HS_HM_PREAMBLE(8);
  HS_KEY_TYPE r1 = HS_XM_GLOBAL_LOAD_L(0);
  HS_KEY_TYPE r2 = HS_XM_GLOBAL_LOAD_L(1);
  HS_KEY_TYPE r3 = HS_XM_GLOBAL_LOAD_L(2);
  HS_KEY_TYPE r4 = HS_XM_GLOBAL_LOAD_L(3);
  HS_KEY_TYPE r5 = HS_XM_GLOBAL_LOAD_L(4);
  HS_KEY_TYPE r6 = HS_XM_GLOBAL_LOAD_L(5);
  HS_KEY_TYPE r7 = HS_XM_GLOBAL_LOAD_L(6);
  HS_KEY_TYPE r8 = HS_XM_GLOBAL_LOAD_L(7);
  HS_KEY_TYPE r9 = HS_XM_GLOBAL_LOAD_L(8);
  HS_KEY_TYPE r10 = HS_XM_GLOBAL_LOAD_L(9);
  HS_KEY_TYPE r11 = HS_XM_GLOBAL_LOAD_L(10);
  HS_KEY_TYPE r12 = HS_XM_GLOBAL_LOAD_L(11);
  HS_KEY_TYPE r13 = HS_XM_GLOBAL_LOAD_L(12);
  HS_KEY_TYPE r14 = HS_XM_GLOBAL_LOAD_L(13);
  HS_KEY_TYPE r15 = HS_XM_GLOBAL_LOAD_L(14);
  HS_KEY_TYPE r16 = HS_XM_GLOBAL_LOAD_L(15);
  HS_CMP_XCHG(r1, r9);
  HS_CMP_XCHG(r5, r13);
  HS_CMP_XCHG(r1, r5);
  HS_CMP_XCHG(r9, r13);
  HS_CMP_XCHG(r3, r11);
  HS_CMP_XCHG(r7, r15);
  HS_CMP_XCHG(r3, r7);
  HS_CMP_XCHG(r11, r15);
  HS_CMP_XCHG(r1, r3);
  HS_CMP_XCHG(r5, r7);
  HS_CMP_XCHG(r9, r11);
  HS_CMP_XCHG(r13, r15);
  HS_CMP_XCHG(r2, r10);
  HS_CMP_XCHG(r6, r14);
  HS_CMP_XCHG(r2, r6);
  HS_CMP_XCHG(r10, r14);
  HS_CMP_XCHG(r4, r12);
  HS_CMP_XCHG(r8, r16);
  HS_CMP_XCHG(r4, r8);
  HS_CMP_XCHG(r12, r16);
  HS_CMP_XCHG(r2, r4);
  HS_CMP_XCHG(r6, r8);
  HS_CMP_XCHG(r10, r12);
  HS_CMP_XCHG(r14, r16);
  HS_CMP_XCHG(r1, r2);
  HS_CMP_XCHG(r3, r4);
  HS_CMP_XCHG(r5, r6);
  HS_CMP_XCHG(r7, r8);
  HS_CMP_XCHG(r9, r10);
  HS_CMP_XCHG(r11, r12);
  HS_CMP_XCHG(r13, r14);
  HS_CMP_XCHG(r15, r16);
  HS_XM_GLOBAL_STORE_L(0, r1);
  HS_XM_GLOBAL_STORE_L(1, r2);
  HS_XM_GLOBAL_STORE_L(2, r3);
  HS_XM_GLOBAL_STORE_L(3, r4);
  HS_XM_GLOBAL_STORE_L(4, r5);
  HS_XM_GLOBAL_STORE_L(5, r6);
  HS_XM_GLOBAL_STORE_L(6, r7);
  HS_XM_GLOBAL_STORE_L(7, r8);
  HS_XM_GLOBAL_STORE_L(8, r9);
  HS_XM_GLOBAL_STORE_L(9, r10);
  HS_XM_GLOBAL_STORE_L(10, r11);
  HS_XM_GLOBAL_STORE_L(11, r12);
  HS_XM_GLOBAL_STORE_L(12, r13);
  HS_XM_GLOBAL_STORE_L(13, r14);
  HS_XM_GLOBAL_STORE_L(14, r15);
  HS_XM_GLOBAL_STORE_L(15, r16);
}

HS_TRANSPOSE_KERNEL_PROTO()
{
  HS_SLAB_GLOBAL_PREAMBLE();
  HS_KEY_TYPE r1 = HS_SLAB_GLOBAL_LOAD(vout, 0);
  HS_KEY_TYPE r2 = HS_SLAB_GLOBAL_LOAD(vout, 1);
  HS_KEY_TYPE r3 = HS_SLAB_GLOBAL_LOAD(vout, 2);
  HS_KEY_TYPE r4 = HS_SLAB_GLOBAL_LOAD(vout, 3);
  HS_KEY_TYPE r5 = HS_SLAB_GLOBAL_LOAD(vout, 4);
  HS_KEY_TYPE r6 = HS_SLAB_GLOBAL_LOAD(vout, 5);
  HS_KEY_TYPE r7 = HS_SLAB_GLOBAL_LOAD(vout, 6);
  HS_KEY_TYPE r8 = HS_SLAB_GLOBAL_LOAD(vout, 7);
  HS_TRANSPOSE_SLAB()
}

//
//
//
