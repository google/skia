/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// NOTE THAT THE SEGMENT TTRK KERNEL IS ENTIRELY DEPENDENT ON THE
// LAYOUT OF THE TTRK KEY.  IF THE TTRK KEY IS ALTERED THEN THIS
// KERNEL WILL NEED TO BE UPDATED
//

#include "tile.h"
#include "kernel_cl_12.h"
#include "raster_builder_cl_12.h" // need meta_in structure
#include "hs/cl/intel/gen8/u64/hs_cl_macros.h"

//
//
//

#define HS_LANE_MASK  (HS_SLAB_WIDTH - 1)

//
// THE BEST TYPE TO ZERO SMEM
//

#define SKC_ZERO_TYPE  ulong
#define SKC_ZERO_WORDS 2

//
// THE ORDER OF COMPONENTS IS:
//
// 0: blocks
// 1: offset
// 2: pk
// 3: rk
//

#if (HS_SLAB_KEYS < 256)

#define SKC_META_TYPE       uint
#define SKC_META_WORDS      1

#define SKC_COMPONENT_TYPE  uchar

#else

#define SKC_META_TYPE       uint2
#define SKC_META_WORDS      2

#define SKC_COMPONENT_TYPE  ushort

#endif

//
//
//

#if ( SKC_TTRK_HI_BITS_COHORT <= 8)
#define SKC_COHORT_TYPE uchar
#else
#define SKC_COHORT_TYPE ushort
#endif

//
//
//

#define SKC_COHORT_ID(row)                      \
  as_uint2(r##row).hi >> SKC_TTRK_HI_OFFSET_COHORT

//
// FIXME -- THIS WILL BREAK IF EITHER THE YX BITS OR OFFSET ARE CHANGED
//

#define SKC_IS_BLOCK(row)                                               \
  ((as_uint2(r##row).lo & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK) == 0)

#define SKC_YX(row,prev)                        \
  (as_uint2(r##row).hi ^ as_uint2(r##prev).hi)

#define SKC_IS_PK(row,prev)                             \
  ((uint)(SKC_YX(row,prev) - 1) < SKC_TTRK_HI_MASK_X)

//
// COHORT   SIZE IS ALWAYS A POWER-OF-TWO
// SUBGROUP SIZE IS ALWAYS A POWER-OF-TWO
//
// COHORT SIZE >= SUBGROUP SIZE
//

#define SKC_COHORT_SIZE           (1<<SKC_TTRK_HI_BITS_COHORT)

#define SKC_ZERO_RATIO            (SKC_ZERO_WORDS / SKC_META_WORDS)
#define SKC_META_ZERO_COUNT       (SKC_COHORT_SIZE * sizeof(SKC_META_TYPE) / sizeof(SKC_ZERO_TYPE))
#define SKC_META_ZERO_REM         (SKC_META_ZERO_COUNT & SKC_BITS_TO_MASK(HS_SLAB_WIDTH_LOG2))

#define SKC_META_COMPONENTS       4
#define SKC_META_COMPONENT_COUNT  (SKC_COHORT_SIZE * sizeof(SKC_META_TYPE) / sizeof(SKC_COMPONENT_TYPE))

//
//
//

__kernel
__attribute__((intel_reqd_sub_group_size(HS_SLAB_WIDTH)))
void
skc_kernel_segment_ttrk(__global HS_KEY_TYPE * SKC_RESTRICT const vout,
                        __global uint        * SKC_RESTRICT const metas)
{
  __local union
  {
    SKC_META_TYPE volatile m[SKC_COHORT_SIZE];
    SKC_ZERO_TYPE          z[SKC_META_ZERO_COUNT];
    SKC_COMPONENT_TYPE     c[SKC_META_COMPONENT_COUNT];
  } shared;

  uint const global_id = get_global_id(0);
  uint const gmem_base = (global_id >> HS_SLAB_WIDTH_LOG2) * HS_SLAB_KEYS;
  uint const gmem_idx  = gmem_base + (global_id & HS_LANE_MASK);
  uint const gmem_off  = (global_id & HS_LANE_MASK) * HS_SLAB_HEIGHT;

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
  uint2 r0    = 0;

  if (gmem_base > 0) {
    // if this is the first key in any slab but the first then it
    // broadcast loads the last key in previous slab
    r0.hi = as_uint2(vout[gmem_base - 1]).hi;
  } else {
    // otherwise broadcast the first key in the first slab
    r0.hi = sub_group_broadcast(as_uint2(r1).hi,0);
    // and mark it as an implicit diff
    if (get_sub_group_local_id() == 0)
      diffs = 1;
  }

  // now shuffle in the last key from the column to the left
  r0.hi = intel_sub_group_shuffle_up(r0.hi,as_uint2(HS_REG_LAST(r)).hi,1);

  // shift away y/x
  SKC_COHORT_TYPE const c0 = r0.hi >> SKC_TTRK_HI_OFFSET_COHORT;

  //
  // EXTRACT ALL COHORT IDS EARLY...
  //
#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                           \
  SKC_COHORT_TYPE c##row = SKC_COHORT_ID(row);

  HS_SLAB_ROWS();

  //
  // DEBUG
  //
#if 0
  if (gmem_base == HS_SLAB_KEYS * 7)
    {
      if (get_sub_group_local_id() == 0)
        printf("\n%llX ",as_ulong(r0));
      else
        printf("%llX ",as_ulong(r0));
#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
      if (get_sub_group_local_id() == 0)        \
        printf("\n%llX ",r##row);               \
      else                                      \
        printf("%llX ",r##row);

      HS_SLAB_ROWS();
    }
#endif

  //
  // CAPTURE ALL CONDITIONS WE CARE ABOUT
  //
  // Diffs must be captured before cohorts
  //
  uint            valid  = 0;
  uint            blocks = 0;
  uint            pks    = 0;
  SKC_COHORT_TYPE c_max  = 0;

  //
  // FIXME -- IT'S UNCLEAR IF SHIFTING THE CONDITION CODE VS. AN
  // EXPLICIT PREDICATE WILL GENERATE THE SAME CODE
  //
#if 0

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  diffs |= ((c##row != c##prev) << prev);

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  blocks |= (SKC_IS_BLOCK(row) << prev);

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  pks |= SKC_IS_PK(row,prev) << prev);

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  valid |= ((r##row != SKC_ULONG_MAX) << prev);

  HS_SLAB_ROWS();

#else

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (c##row != c##prev)                        \
    diffs |= 1<<prev;

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (SKC_IS_BLOCK(row))                        \
    blocks |= 1<<prev;

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (SKC_IS_PK(row,prev))                      \
    pks |= 1<<prev;

  HS_SLAB_ROWS();

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (r##row != SKC_ULONG_MAX) {                \
    valid |= 1<<prev;                           \
    c_max  = max(c_max,c##row);                 \
  }

  HS_SLAB_ROWS();

#endif

  //
  // TRANSPOSE THE SLAB AND STORE IT
  //
  HS_TRANSPOSE_SLAB();

  // the min cohort is the first key in the slab
  uint const c_min = sub_group_broadcast(c1,0);

  // the max cohort is the max across all lanes
  c_max = sub_group_reduce_max(c_max);

#if 0 // REMOVE ME LATER
  if (get_sub_group_local_id() == 0)
    printf("%3u : ( %3u , %3u )\n",
           get_global_id(0)>>HS_SLAB_WIDTH_LOG2,c_min,c_max);
#endif

  //
  // ZERO SMEM
  //
  // zero only the meta info for the cohort ids found in this slab
  //
#if   (SKC_ZERO_WORDS >= SKC_META_WORDS)
  uint       zz     = ((c_min / SKC_ZERO_RATIO) & ~HS_LANE_MASK) + get_sub_group_local_id();
  uint const zz_max = (c_max + SKC_ZERO_RATIO - 1) / SKC_ZERO_RATIO;

  for (; zz<=zz_max; zz+=HS_SLAB_WIDTH)
    shared.z[zz] = 0;
#else
  // ERROR -- it's highly unlikely that the zero type is smaller than
  // the meta type
#error("Unsupported right now...")
#endif

  //
  // ACCUMULATE AND STORE META INFO
  //
  uint const    valid_blocks = valid & blocks;
  uint const    valid_pks    = valid & pks & ~diffs;
  SKC_META_TYPE meta         = ( 0 );

#define SKC_META_LOCAL_ADD(meta)                \
  atomic_add(shared.m+HS_REG_LAST(c),meta);

#define SKC_META_LOCAL_STORE(meta,prev)         \
  shared.m[c##prev] = meta;

  // note this is purposefully off by +1
#define SKC_META_RESET(meta,curr)               \
  meta = ((gmem_off + curr) << 8);

#if 0

  // FIXME -- this can be tweaked to shift directly
#define SKC_META_ADD(meta,prev,blocks,pks,rks)  \
  meta += ((((blocks >> prev) & 1)      ) |     \
           (((pks    >> prev) & 1) << 16) |     \
           (((rks    >> prev) & 1) << 24));

#else

#define SKC_META_ADD(meta,prev,blocks,pks,rks)  \
  if (blocks & (1<<prev))                       \
    meta += 1;                                  \
  if (pks    & (1<<prev))                       \
    meta += 1<<16;                              \
  if (rks    & (1<<prev))                       \
    meta += 1<<24;

#endif

#undef  HS_SLAB_ROW
#define HS_SLAB_ROW(row,prev)                   \
  if (diffs & (1<<prev)) {                      \
    SKC_META_LOCAL_STORE(meta,prev);            \
    SKC_META_RESET(meta,row);                   \
  }                                             \
  SKC_META_ADD(meta,prev,                       \
               valid_blocks,                    \
               valid_pks,                       \
               valid);

  HS_SLAB_ROWS();

  //
  // ATOMICALLY ADD THE CARRIED OUT METAS
  //
#if 0 // BUG
  if ((valid & (1<<(HS_SLAB_HEIGHT-1))) && (meta != 0))
    SKC_META_LOCAL_ADD(meta);
#else
  if (meta != 0)
    SKC_META_LOCAL_ADD(meta);
#endif

  //
  // NOW ATOMICALLY ADD ALL METAS TO THE GLOBAL META TABLE
  //

  // convert the slab offset to an extent offset
  bool const is_offset = (get_sub_group_local_id() & 3) == 1;
  uint const adjust    = is_offset ? gmem_base - 1 : 0;

  //
  // only process the meta components found in this slab
  //
  uint const cc_min = c_min * SKC_META_COMPONENTS;
  uint const cc_max = c_max * SKC_META_COMPONENTS + SKC_META_COMPONENTS - 1;
  uint       cc     = (cc_min & ~HS_LANE_MASK) + get_sub_group_local_id();

  if ((cc >= cc_min) && (cc <= cc_max))
    {
      uint const c = shared.c[cc];

      if (c != 0)
        atomic_add(metas+cc,c+adjust);
    }

  cc += HS_SLAB_WIDTH;

  for (; cc<=cc_max; cc+=HS_SLAB_WIDTH)
    {
      uint const c = shared.c[cc];

      if (c != 0)
        atomic_add(metas+cc,c+adjust);
    }
}

//
//
//
