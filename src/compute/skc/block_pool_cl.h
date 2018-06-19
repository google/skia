/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_BLOCK_POOL
#define SKC_ONCE_BLOCK_POOL

//
//
//

#include "types.h"

//
//
//

union skc_block_pool_size
{
  skc_uint3   u32v3;

  struct {
    skc_uint  pool_size; // number of blocks
    skc_uint  ring_pow2; // rounded-up pow2 of pool_size
    skc_uint  ring_mask; // ring_pow2 - 1
  };
};

//
//
//

union skc_block_pool_atomic
{
  skc_uint2  u32v2;

  skc_uint   u32a2[2];

  struct {
    skc_uint reads;
    skc_uint writes;
  };
};

#define SKC_BP_ATOMIC_OFFSET_READS   0
#define SKC_BP_ATOMIC_OFFSET_WRITES  1

//
//
//

#endif

//
//
//
