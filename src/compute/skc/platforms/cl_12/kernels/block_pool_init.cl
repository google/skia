/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "kernel_cl_12.h"

//
// BEST TO RUN THESE ON AN OUT-OF-ORDER CQ
//

__kernel
SKC_BP_INIT_IDS_KERNEL_ATTRIBS
void
skc_kernel_block_pool_init_ids(__global uint * const ids, uint const bp_size)
{
  uint const gid = get_global_id(0);

  //
  // FIXME -- TUNE FOR ARCH -- evaluate if it's much faster to
  // accomplish this with fewer threads and using either IPC and/or
  // vector stores -- it should be on certain architectures!
  //

  //
  // initialize pool with sequence
  //
  if (gid < bp_size)
    ids[gid] = gid * SKC_DEVICE_SUBBLOCKS_PER_BLOCK;
}

//
//
//

__kernel
SKC_BP_INIT_ATOMICS_KERNEL_ATTRIBS
void
skc_kernel_block_pool_init_atomics(__global uint * const bp_atomics, uint const bp_size)
{
  // the version test is to squelch a bug with the Intel OpenCL CPU
  // compiler declaring it supports the cl_intel_subgroups extension
#if defined(cl_intel_subgroups) || defined (cl_khr_subgroups)
  uint const tid = get_sub_group_local_id();
#else
  uint const tid = get_local_id(0);
#endif

  //
  // launch two threads and store [ 0, bp_size ]
  //
  bp_atomics[tid] = tid * bp_size;
}

//
//
//
