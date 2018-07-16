/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// NOTE THAT THE SEGMENT TTCK KERNEL IS ENTIRELY DEPENDENT ON THE
// LAYOUT OF THE TTCK KEY.  IF THE TTCK KEY IS ALTERED THEN THIS
// KERNEL WILL NEED TO BE UPDATED
//

#include "tile.h"
#include "atomic_cl.h"
#include "kernel_cl_12.h"
#include "hs/cl/intel/gen8/u64/hs_cl_macros.h"

//
//
//

#define HS_LANE_MASK (HS_SLAB_WIDTH - 1)

//
//
//

#define SKC_YX_NEQ(row,prev)                \
  (((as_uint2(r##row).hi ^ as_uint2(r##prev).hi) & SKC_TTCK_HI_MASK_YX) != 0)

//
//
//

__kernel
__attribute__((intel_reqd_sub_group_size(HS_SLAB_WIDTH)))
void
skc_kernel_segment_ttck(__global HS_KEY_TYPE              * SKC_RESTRICT const vout,
                        __global uint                     * SKC_RESTRICT const indices,
                        __global SKC_ATOMIC_UINT volatile * SKC_RESTRICT const atomics)
{
  uint const global_id = get_global_id(0);
  uint const gmem_base = (global_id >> HS_SLAB_WIDTH_LOG2) * HS_SLAB_KEYS;
  uint const gmem_idx  = gmem_base + (global_id & HS_LANE_MASK);
  uint const lane_idx  = gmem_base + (global_id & HS_LANE_MASK) * HS_SLAB_HEIGHT;

  //
  // LOAD ALL THE ROWS
  //
#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                                           \
  HS_KEY_TYPE const r##row = (vout + gmem_idx)[prev * HS_SLAB_WIDTH];

  HS_SLAB_ROWS();

  //
  // LOAD LAST REGISTER FROM COLUMN TO LEFT
  //
  uint  diffs = 0;
  uint2 r0    = r1;

  if (gmem_base > 0) {
    // if this is the first key in any slab but the first then it
    // broadcast loads the last key in previous slab
    r0.hi = as_uint2(vout[gmem_base - 1]).hi;
  } else if (get_sub_group_local_id() == 0) {
    // if this is the first lane in the first slab
    diffs = 1;
  }

  // now shuffle in the last key from the column to the left
  r0.hi = intel_sub_group_shuffle_up(r0.hi,as_uint2(HS_REG_LAST(r)).hi,1);

  //
  // FIND ALL DIFFERENCES IN SLAB
  //
  uint valid = 0;

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  valid |= ((r##row != SKC_ULONG_MAX) << prev);

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  diffs |= (SKC_YX_NEQ(row,prev) << prev);

  HS_SLAB_ROWS();

  //
  // SUM UP THE DIFFERENCES
  //
  uint const valid_diffs = valid & diffs;
  uint const count       = popcount(valid_diffs);
  uint const inclusive   = sub_group_scan_inclusive_add(count);
  uint const exclusive   = inclusive - count;

  //
  // RESERVE SPACE IN THE INDICES ARRAY
  //
  uint next = 0;

  if (get_sub_group_local_id() == HS_SLAB_WIDTH-1)
    next = atomic_add(atomics+1,inclusive); // FIXME -- need a symbolic offset

  // distribute base across subgroup
  next = exclusive + sub_group_broadcast(next,HS_SLAB_WIDTH-1);

  //
  // STORE THE INDICES
  //
#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (valid_diffs & (1 << prev))                \
    indices[next++] = lane_idx + prev;

  HS_SLAB_ROWS();

  //
  // TRANSPOSE THE SLAB AND STORE IT
  //
  HS_TRANSPOSE_SLAB();
}

//
//
//
