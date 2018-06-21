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

#include "tile.h"
#include "block.h"
#include "raster.h"
#include "common.h"
#include "atomic_cl.h"
#include "block_pool_cl.h"
#include "kernel_cl_12.h"

//
//
//

#define SKC_RASTERS_RECLAIM_SUBGROUP_SIZE_MASK (SKC_RASTERS_RECLAIM_SUBGROUP_SIZE - 1)

#define SKC_RASTERS_RECLAIM_SUBGROUP_WORDS     (SKC_RASTERS_RECLAIM_SUBGROUP_SIZE * SKC_RASTERS_RECLAIM_LOCAL_ELEMS)

#define SKC_RASTERS_RECLAIM_X                  (SKC_DEVICE_BLOCK_DWORDS / SKC_RASTERS_RECLAIM_SUBGROUP_WORDS)

//
//
//

#if   ( SKC_RASTERS_RECLAIM_X == 1 )
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_1()
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST  0

#elif ( SKC_RASTERS_RECLAIM_X == 2 )
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_2()
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST  1

#elif ( SKC_RASTERS_RECLAIM_X == 4 )
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_4()
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST  3

#elif ( SKC_RASTERS_RECLAIM_X == 8 )
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_8()
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST  7

#elif ( SKC_RASTERS_RECLAIM_X == 16)
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_16()
#define SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST  15

#else
#error "MISSING SKC_RASTERS_RECLAIM_X"
#endif

#if    ( SKC_PREFIX_SUBGROUP_SIZE == SKC_RASTERS_RECLAIM_SUBGROUP_SIZE )

#define SKC_RASTERS_RECLAIM_STRIDE_H(L)              (L)
#define SKC_RASTERS_RECLAIM_STRIDE_V_LO(I)           (I * 2 * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)
#define SKC_RASTERS_RECLAIM_STRIDE_V_HI(I)           (SKC_RASTERS_RECLAIM_STRIDE_V_LO(I) + SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)

#elif  ( SKC_PREFIX_SUBGROUP_SIZE >  SKC_RASTERS_RECLAIM_SUBGROUP_SIZE ) // same as above when ratio equals 1

#define SKC_RASTERS_RECLAIM_SUBGROUP_RATIO           (SKC_PREFIX_SUBGROUP_SIZE / SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)
#define SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_MASK      (SKC_RASTERS_RECLAIM_SUBGROUP_RATIO - 1)
#define SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_SCALE(I)  ((I / SKC_RASTERS_RECLAIM_SUBGROUP_RATIO) * 2 * SKC_RASTERS_RECLAIM_SUBGROUP_RATIO + \
                                                      (I & SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_MASK))

#define SKC_RASTERS_RECLAIM_STRIDE_H(L)              (L)
#define SKC_RASTERS_RECLAIM_STRIDE_V_LO(I)           (SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_SCALE(I) * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)
#define SKC_RASTERS_RECLAIM_STRIDE_V_HI(I)           (SKC_RASTERS_RECLAIM_STRIDE_V_LO(I) + SKC_RASTERS_RECLAIM_SUBGROUP_RATIO * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)

#elif  ( SKC_PREFIX_SUBGROUP_SIZE <  SKC_RASTERS_RECLAIM_SUBGROUP_SIZE ) // same as above when ratio equals 1

#define SKC_RASTERS_RECLAIM_SUBGROUP_RATIO           (SKC_RASTERS_RECLAIM_SUBGROUP_SIZE / SKC_PREFIX_SUBGROUP_SIZE)
#define SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_MASK      (SKC_RASTERS_RECLAIM_SUBGROUP_SIZE / SKC_RASTERS_RECLAIM_SUBGROUP_RATIO - 1) // equal to prefix subgroup mask

#define SKC_RASTERS_RECLAIM_STRIDE_H(L)              (((L) & ~SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_MASK) * 2 + ((L) & SKC_RASTERS_RECLAIM_SUBGROUP_RATIO_MASK))
#define SKC_RASTERS_RECLAIM_STRIDE_V_LO(I)           (I * 2 * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)
#define SKC_RASTERS_RECLAIM_STRIDE_V_HI(I)           (SKC_RASTERS_RECLAIM_STRIDE_V_LO(I) + SKC_RASTERS_RECLAIM_SUBGROUP_SIZE / SKC_RASTERS_RECLAIM_SUBGROUP_RATIO)

#endif

//
// FIXME -- slate these for replacement
//

#define SKC_BROADCAST(E,S,I)                                            \
  sub_group_broadcast(E,S - I * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)

#define SKC_BROADCAST_LAST_HELPER(E,I)                          \
  sub_group_broadcast(E,SKC_RASTERS_RECLAIM_SUBGROUP_SIZE - 1)

#define SKC_BROADCAST_LAST(E,I)                 \
  SKC_BROADCAST_LAST_HELPER(E,I)

//
// COMPILE-TIME PREDICATES
//

#define SKC_RASTERS_RECLAIM_ELEM_GTE(X,I)                       \
  SKC_GTE_MACRO(X,(I+1) * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)

#define SKC_RASTERS_RECLAIM_ELEM_IN_RANGE(X,I)                          \
  (skc_bool)SKC_GTE_MACRO(X, I   * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE) && \
  (skc_bool)SKC_LT_MACRO(X,(I+1) * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE)

#define SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)          \
  SKC_RASTERS_RECLAIM_ELEM_GTE(SKC_RASTER_HEAD_DWORDS,I)

#define SKC_RASTERS_RECLAIM_PARTIALLY_HEADER(I)                 \
  SKC_RASTERS_RECLAIM_ELEM_IN_RANGE(SKC_RASTER_HEAD_DWORDS,I)

//
// RUN-TIME PREDICATES
//

#define SKC_RASTERS_RECLAIM_IS_HEADER(I)                                \
  (get_sub_group_local_id() + I * SKC_RASTERS_RECLAIM_SUBGROUP_SIZE < SKC_RASTER_HEAD_DWORDS)

//
// FIXME -- THIS BITFIELD SCAN APPROACH CAN BE PARAMETERIZED FOR ALL
// POSSIBLE PRACTICAL POWER-OF-TWO SUBGROUP AND SUBBLOCKS-PER-BLOCK
// COMBOS (NOT NECESSARILY POW2)
//
// FOR WIDER SUBGROUPS WITH BIG BLOCKS, WE WILL WANT TO USE A VECTOR
// UINT TYPE INSTEAD OF A ULONG.
//

#define SKC_RASTERS_RECLAIM_PACKED_COUNT_BITS     SKC_RASTERS_RECLAIM_SUBGROUP_SIZE_LOG2
#define SKC_RASTERS_RECLAIM_PACKED_COUNT_DECLARE  skc_uint

//
//
//

#define SKC_RASTERS_RECLAIM_PACKED_COUNT_MASK  SKC_BITS_TO_MASK(SKC_RASTERS_RECLAIM_PACKED_COUNT_BITS)

#define SKC_RASTERS_RECLAIM_PACKED_COUNT_IS_BLOCK(E,I)          \
  (((E) & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK)                  \
   ? 0 : (1u << SKC_RASTERS_RECLAIM_PACKED_COUNT_BITS * I))

#define SKC_RASTERS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(S,C)        \
  S = sub_group_scan_exclusive_add(C)

#define SKC_RASTERS_RECLAIM_PACKED_COUNT_GET(C,I)                       \
  (((C) >> (SKC_RASTERS_RECLAIM_PACKED_COUNT_BITS * I)) & SKC_RASTERS_RECLAIM_PACKED_COUNT_MASK)

//
//
//

struct skc_reclaim
{
  skc_raster_h aN[SKC_RECLAIM_ARRAY_SIZE];
};

__kernel
SKC_RASTERS_RECLAIM_KERNEL_ATTRIBS
void
skc_kernel_rasters_reclaim(__global skc_block_id_t          * const bp_ids,      // block pool ids ring
                           __global skc_uint                * const bp_elems,    // block pool blocks
                           __global skc_uint       volatile * const bp_atomics,  // read/write atomics
                           skc_uint                           const bp_mask,     // pow2 modulo mask for block pool ring
                           __global skc_block_id_t const    * const map,         // raster host-to-device map
                           struct   skc_reclaim               const reclaim)     // array of host raster ids
{
#if (__OPENCL_VERSION__ < 200)
  skc_uint const reclaim_stride = get_num_sub_groups();
#else
  skc_uint const reclaim_stride = get_enqueued_num_sub_groups(); // 2.0 supports non-uniform workgroups
#endif
  skc_uint       reclaim_idx    = get_group_id(0) * reclaim_stride + get_sub_group_id();

#if 0
  //
  // NOTE -- FOR NOW, THIS KERNEL ALWAYS LAUNCHES FIXED SIZE GRIDS BUT
  // WE MIGHT WANT TO HAVE THE GRID LIMIT ITSELF TO A FRACTIONAL
  // MULTIPROCESSOR IN ORDER TO MINIMIZE THE IMPACT OF A LARGE
  // RECLAMATION JOB ON THE REST OF THE PIPELINE.
  //
  for (; reclaim_idx < SKC_RECLAIM_ARRAY_SIZE; reclaim_idx+=reclaim_stride)
#endif
    {
      // get host raster id
      skc_raster_h const raster = reclaim.aN[reclaim_idx];

      // get block id of raster header
      skc_block_id_t     id     = map[raster];

      //
      // load all of the head block ttxk.lo keys into registers
      //
      // FIXME -- this pattern lends itself to using the higher
      // performance Intel GEN block load instructions
      //
      skc_uint const head_id = id * SKC_DEVICE_SUBBLOCK_WORDS + SKC_RASTERS_RECLAIM_STRIDE_H(get_sub_group_local_id());

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      skc_uint h##I = bp_elems[head_id + SKC_RASTERS_RECLAIM_STRIDE_V_LO(I)];

      SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

      //
      // pick out count.nodes and count.prims from the header
      //
      // load raster header counts -- we only need the blocks and
      // nodes words the keys are doublewords.
      //
      // FIXME -- this can be made portable with compile-time macro expansion
      //
      skc_uint count_blocks = sub_group_broadcast(h0,0); // SKC_RASTER_HEAD_OFFSET_COUNTS_NODES
      skc_uint count_nodes  = sub_group_broadcast(h0,1); // SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS

#if 0
      if (get_sub_group_local_id() == 0) {
        printf("reclaim rasters: %u / %u / %5u / %5u\n",raster,id,count_blocks,count_nodes);
      }
#endif
      //
      // acquire a span in the block pool ids ring for reclaimed ids
      //
      skc_uint bp_ids_base = 0;

      if (get_sub_group_local_id() == 0) {
        bp_ids_base = SKC_ATOMIC_ADD_GLOBAL_RELAXED_SUBGROUP(bp_atomics+SKC_BP_ATOMIC_OFFSET_WRITES,count_blocks);
      }

      bp_ids_base = sub_group_broadcast(bp_ids_base,0);

      //
      // mask off everything but the block id
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
      if (!SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)) {    \
        h##I = h##I & SKC_TTXK_LO_MASK_ID;              \
      }

      SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

      //
      // swap current id with next
      //
      if (get_sub_group_local_id() == SKC_RASTERS_RECLAIM_SUBGROUP_SIZE - 1)
        {
          skc_block_id_t const next = SKC_CONCAT(h,SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST);

          SKC_CONCAT(h,SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST) = id;

          id = next;
#if 0
          printf("rasters next = %u\n",id);
#endif
        }

#if 0
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                 \
        printf("%08X %u\n",h##I,h##I);

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();
#endif
      
#if 0
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
      if (!SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)) {    \
        printf("%08X\n",h##I);                          \
      }

      SKC_RASTERS_RECLAIM_BLOCK_EXPAND();
#endif

      //
      // - we'll skip subgroups that are entirely header
      //
      // - but we need to mark any header elements that partially fill
      //   a subgroup as subblocks
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
      if (!SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)) {    \
        if (SKC_RASTERS_RECLAIM_PARTIALLY_HEADER(I)) {  \
          if (SKC_RASTERS_RECLAIM_IS_HEADER(I)) {       \
            h##I = SKC_UINT_MAX;                        \
          }                                             \
        }                                               \
      }

      SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

      {
        //
        // count reclaimable blocks in each lane
        //
        SKC_RASTERS_RECLAIM_PACKED_COUNT_DECLARE packed_count = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        if (!SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)) {                  \
          packed_count |= SKC_RASTERS_RECLAIM_PACKED_COUNT_IS_BLOCK(h##I,I); \
        }

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

        //
        // scan to find index of each block
        //
        SKC_RASTERS_RECLAIM_PACKED_COUNT_DECLARE packed_index = ( 0 );

        SKC_RASTERS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(packed_index,packed_count);

        //
        // store blocks back to ring
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        if (!SKC_RASTERS_RECLAIM_ENTIRELY_HEADER(I)) {                  \
          skc_uint const index      = SKC_RASTERS_RECLAIM_PACKED_COUNT_GET(packed_index,I); \
          skc_uint const count      = SKC_RASTERS_RECLAIM_PACKED_COUNT_GET(packed_count,I); \
          skc_uint const bp_ids_idx = (bp_ids_base + index) & bp_mask;  \
          if (count > 0) {                                              \
            bp_ids[bp_ids_idx] = h##I;                                  \
          }                                                             \
          skc_uint const total = index + count;                         \
          bp_ids_base += sub_group_broadcast(total,SKC_RASTERS_RECLAIM_SUBGROUP_SIZE-1); \
        }

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();
      }

      // printf("R %7u ! %u\n",bp_ids_idx,h##I);
            
      //
      // we're done if it was just the header
      //
      if (count_nodes == 0)
        return;

      //
      // otherwise, walk the nodes
      //
      do {
        // id of next block is in last lane
        id = sub_group_broadcast(id,SKC_RASTERS_RECLAIM_SUBGROUP_SIZE-1);

        //
        // load all of the node block ttxk.lo keys into registers
        //
        // FIXME -- this pattern lends itself to using the higher
        // performance Intel GEN block load instructions
        //
        skc_uint const node_id = id * SKC_DEVICE_SUBBLOCK_WORDS + SKC_RASTERS_RECLAIM_STRIDE_H(get_sub_group_local_id());

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        skc_uint n##I = bp_elems[node_id + SKC_RASTERS_RECLAIM_STRIDE_V_LO(I)];

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

        //
        // mask off everything but the block id
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                 \
        n##I = n##I & SKC_TTXK_LO_MASK_ID;

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

        //
        // swap current id with next
        //
        if (get_sub_group_local_id() == SKC_RASTERS_RECLAIM_SUBGROUP_SIZE - 1)
          {
            skc_block_id_t const next = SKC_CONCAT(n,SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST);

            SKC_CONCAT(n,SKC_RASTERS_RECLAIM_BLOCK_EXPAND_I_LAST) = id;

            id = next;
#if 0
            printf("rasters next = %u\n",id);            
#endif
          }

#if 0
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                 \
        printf("%08X %u\n",n##I,n##I);

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();
#endif

        //
        // count reclaimable blocks in each lane
        //
        SKC_RASTERS_RECLAIM_PACKED_COUNT_DECLARE packed_count = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        packed_count |= SKC_RASTERS_RECLAIM_PACKED_COUNT_IS_BLOCK(n##I,I);

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

        //
        // scan to find index of each block
        //
        SKC_RASTERS_RECLAIM_PACKED_COUNT_DECLARE packed_index = ( 0 );

        SKC_RASTERS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(packed_index,packed_count);

        //
        // store blocks back to ring
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                                       \
          skc_uint const index      = SKC_RASTERS_RECLAIM_PACKED_COUNT_GET(packed_index,I); \
          skc_uint const count      = SKC_RASTERS_RECLAIM_PACKED_COUNT_GET(packed_count,I); \
          skc_uint const bp_ids_idx = (bp_ids_base + index) & bp_mask;  \
          if (count > 0) {                                              \
            bp_ids[bp_ids_idx] = n##I;                                  \
          }                                                             \
          skc_uint const total = index + count;                         \
          bp_ids_base += sub_group_broadcast(total,SKC_RASTERS_RECLAIM_SUBGROUP_SIZE-1); \
        }

        SKC_RASTERS_RECLAIM_BLOCK_EXPAND();

        // printf("R %7u ! %u\n",bp_ids_idx,n##I);
        
        // any more nodes?
      } while (--count_nodes > 0);
    }
}

//
//
//
