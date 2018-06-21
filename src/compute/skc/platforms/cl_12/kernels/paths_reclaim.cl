/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// FIXME -- a pre-allocation step could load the path header quads and
// total up the number of blocks in the workgroup or subgroup
// minimizing the number of later atomics adds.
//

#include "block.h"
#include "path.h"
#include "common.h"
#include "atomic_cl.h"
#include "block_pool_cl.h"
#include "kernel_cl_12.h"

//
//
//

#define SKC_PATHS_RECLAIM_SUBGROUP_SIZE_MASK (SKC_PATHS_RECLAIM_SUBGROUP_SIZE - 1)

#define SKC_PATHS_RECLAIM_SUBGROUP_ELEMS     (SKC_PATHS_RECLAIM_SUBGROUP_SIZE * SKC_PATHS_RECLAIM_LOCAL_ELEMS)

#define SKC_PATHS_RECLAIM_X                  (SKC_DEVICE_BLOCK_WORDS / SKC_PATHS_RECLAIM_SUBGROUP_ELEMS)

//
//
//

#if   ( SKC_PATHS_RECLAIM_X == 1 )
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_1()
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST  0

#elif ( SKC_PATHS_RECLAIM_X == 2 )
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_2()
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST  1

#elif ( SKC_PATHS_RECLAIM_X == 4 )
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_4()
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST  3

#elif ( SKC_PATHS_RECLAIM_X == 8 )
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_8()
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST  7

#elif ( SKC_PATHS_RECLAIM_X == 16)
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND()       SKC_EXPAND_16()
#define SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST  15

#else
#error "MISSING SKC_PATHS_RECLAIM_X"
#endif

//
// FIXME -- slate these for replacement
//

#define SKC_BROADCAST(E,S,I)                                            \
  sub_group_broadcast(E,S - I * SKC_PATHS_RECLAIM_SUBGROUP_SIZE)

#define SKC_BROADCAST_LAST_HELPER(E,I)                          \
  sub_group_broadcast(E,SKC_PATHS_RECLAIM_SUBGROUP_SIZE - 1)

#define SKC_BROADCAST_LAST(E,I)                 \
  SKC_BROADCAST_LAST_HELPER(E,I)

//
// COMPILE-TIME PREDICATES
//

#define SKC_PATHS_RECLAIM_ELEM_GTE(X,I)                         \
  SKC_GTE_MACRO(X,(I+1) * SKC_PATHS_RECLAIM_SUBGROUP_SIZE)

#define SKC_PATHS_RECLAIM_ELEM_IN_RANGE(X,I)                            \
  (skc_bool)SKC_GTE_MACRO(X, I   * SKC_PATHS_RECLAIM_SUBGROUP_SIZE) &&  \
  (skc_bool)SKC_LT_MACRO(X,(I+1) * SKC_PATHS_RECLAIM_SUBGROUP_SIZE)

#define SKC_PATHS_RECLAIM_ENTIRELY_HEADER(I)            \
  SKC_PATHS_RECLAIM_ELEM_GTE(SKC_PATH_HEAD_WORDS,I)

#define SKC_PATHS_RECLAIM_PARTIALLY_HEADER(I)                   \
  SKC_PATHS_RECLAIM_ELEM_IN_RANGE(SKC_PATH_HEAD_WORDS,I)

//
// RUN-TIME PREDICATES
//

#define SKC_PATHS_RECLAIM_IS_HEADER(I)                                  \
  (get_sub_group_local_id() + I * SKC_PATHS_RECLAIM_SUBGROUP_SIZE < SKC_PATH_HEAD_WORDS)

//
// FIXME -- THIS BITFIELD SCAN APPROACH CAN BE PARAMETERIZED FOR ALL
// POSSIBLE PRACTICAL POWER-OF-TWO SUBGROUP AND SUBBLOCKS-PER-BLOCK
// COMBOS (NOT NECESSARILY POW2)
//
// FOR WIDER SUBGROUPS WITH BIG BLOCKS, WE WILL WANT TO USE A VECTOR
// UINT TYPE INSTEAD OF A ULONG.
//

#define SKC_PATHS_RECLAIM_PACKED_COUNT_BITS     SKC_PATHS_RECLAIM_SUBGROUP_SIZE_LOG2
#define SKC_PATHS_RECLAIM_PACKED_COUNT_DECLARE  skc_uint

//
//
//

#define SKC_PATHS_RECLAIM_PACKED_COUNT_MASK  SKC_BITS_TO_MASK(SKC_PATHS_RECLAIM_PACKED_COUNT_BITS)

#define SKC_PATHS_RECLAIM_PACKED_COUNT_IS_BLOCK(E,I)            \
  (((E) & SKC_DEVICE_SUBBLOCKS_PER_BLOCK_MASK)                  \
   ? 0 : (1u << SKC_PATHS_RECLAIM_PACKED_COUNT_BITS * I))

#define SKC_PATHS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(S,C)  \
  S = sub_group_scan_exclusive_add(C)

#define SKC_PATHS_RECLAIM_PACKED_COUNT_GET(C,I)                         \
  (((C) >> (SKC_PATHS_RECLAIM_PACKED_COUNT_BITS * I)) & SKC_PATHS_RECLAIM_PACKED_COUNT_MASK)

//
//
//

struct skc_reclaim
{
  skc_path_h aN[SKC_RECLAIM_ARRAY_SIZE];
};

__kernel
SKC_PATHS_RECLAIM_KERNEL_ATTRIBS
void
skc_kernel_paths_reclaim(__global skc_block_id_t          * const bp_ids,      // block pool ids ring
                         __global skc_uint                * const bp_elems,    // block pool blocks
                         __global skc_uint       volatile * const bp_atomics,  // read/write atomics
                         skc_uint                           const bp_mask,     // pow2 modulo mask for block pool ring
                         __global skc_block_id_t const    * const map,         // path host-to-device map
                         struct   skc_reclaim               const reclaim)     // array of host path ids
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
      // get host path id
      skc_path_h const path = reclaim.aN[reclaim_idx];

      // get the path header block from the map
      skc_block_id_t   id   = map[path];

      //
      // blindly load all of the head elements into registers
      //
      skc_uint const head_idx = id * SKC_DEVICE_SUBBLOCK_WORDS + get_sub_group_local_id();

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      skc_uint h##I = bp_elems[head_idx + I * SKC_PATHS_RECLAIM_SUBGROUP_SIZE];

      SKC_PATHS_RECLAIM_BLOCK_EXPAND();

      //
      // pick out count.nodes and count.prims from the header
      //
      skc_uint count_blocks, count_nodes;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      if (SKC_PATHS_RECLAIM_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_BLOCKS,I)) { \
        count_blocks = SKC_BROADCAST(h##I,SKC_PATH_HEAD_OFFSET_BLOCKS,I); \
      }                                                                 \
      if (SKC_PATHS_RECLAIM_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_NODES,I)) { \
        count_nodes  = SKC_BROADCAST(h##I,SKC_PATH_HEAD_OFFSET_NODES,I); \
      }

      SKC_PATHS_RECLAIM_BLOCK_EXPAND();

#if 0
      if (get_sub_group_local_id() == 0) {
        printf("reclaim paths:   %9u / %5u / %5u\n",path,count_blocks,count_nodes);
      }
#endif

      //
      // acquire a span in the block pool ids ring for reclaimed ids
      //
      // FIXME count_blocks and atomic add can be done in same lane
      //
      skc_uint bp_ids_base = 0;

      if (get_sub_group_local_id() == 0) {
        bp_ids_base = SKC_ATOMIC_ADD_GLOBAL_RELAXED_SUBGROUP(bp_atomics+SKC_BP_ATOMIC_OFFSET_WRITES,count_blocks);

#if 0
        printf("paths: bp_ids_base = %u\n",bp_ids_base);
#endif
      }

      bp_ids_base = sub_group_broadcast(bp_ids_base,0);

      //
      // shift away the tagged block id's tag
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
      if (!SKC_PATHS_RECLAIM_ENTIRELY_HEADER(I)) {      \
        h##I = h##I >> SKC_TAGGED_BLOCK_ID_BITS_TAG;    \
      }

      SKC_PATHS_RECLAIM_BLOCK_EXPAND();

      //
      // swap current id with next
      //
      if (get_sub_group_local_id() == SKC_PATHS_RECLAIM_SUBGROUP_SIZE - 1)
        {
          skc_block_id_t const next = SKC_CONCAT(h,SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST);

          SKC_CONCAT(h,SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST) = id;

          id = next;
        }

      //
      // - we'll skip subgroups that are entirely header
      //
      // - but we need to mark any header elements that partially fill
      //   a subgroup as invalid tagged block ids
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
      if (!SKC_PATHS_RECLAIM_ENTIRELY_HEADER(I)) {      \
        if (SKC_PATHS_RECLAIM_PARTIALLY_HEADER(I)) {    \
          if (SKC_PATHS_RECLAIM_IS_HEADER(I)) {         \
            h##I = SKC_TAGGED_BLOCK_ID_INVALID;         \
          }                                             \
        }                                               \
      }

      SKC_PATHS_RECLAIM_BLOCK_EXPAND();

      {
        //
        // count reclaimable blocks in each lane
        //
        SKC_PATHS_RECLAIM_PACKED_COUNT_DECLARE packed_count = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        if (!SKC_PATHS_RECLAIM_ENTIRELY_HEADER(I)) {                    \
          packed_count |= SKC_PATHS_RECLAIM_PACKED_COUNT_IS_BLOCK(h##I,I); \
        }

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        //
        // scan to find index of each block
        //
        SKC_PATHS_RECLAIM_PACKED_COUNT_DECLARE packed_index = ( 0 );

        SKC_PATHS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(packed_index,packed_count);

        //
        // store blocks back to ring
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        if (!SKC_PATHS_RECLAIM_ENTIRELY_HEADER(I)) {                    \
          skc_uint const index      = SKC_PATHS_RECLAIM_PACKED_COUNT_GET(packed_index,I); \
          skc_uint const count      = SKC_PATHS_RECLAIM_PACKED_COUNT_GET(packed_count,I); \
          skc_uint const bp_ids_idx = (bp_ids_base + index) & bp_mask;  \
          if (count > 0) {                                              \
            bp_ids[bp_ids_idx] = h##I;                                  \
          }                                                             \
          skc_uint const total = index + count;                         \
          bp_ids_base += sub_group_broadcast(total,SKC_PATHS_RECLAIM_SUBGROUP_SIZE-1); \
        }

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        // printf("P %7u ! %u\n",bp_ids_idx,h##I);
      }

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
        id = sub_group_broadcast(id,SKC_PATHS_RECLAIM_SUBGROUP_SIZE-1);

        // get index of each element
        skc_uint const node_idx = id * SKC_DEVICE_SUBBLOCK_WORDS + get_sub_group_local_id();

        //
        // blindly load all of the node elements into registers
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        skc_uint n##I = bp_elems[node_idx + I * SKC_PATHS_RECLAIM_SUBGROUP_SIZE];

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        //
        // shift away the tagged block id's tag
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                         \
        n##I = n##I >> SKC_TAGGED_BLOCK_ID_BITS_TAG;

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        //
        // swap current id with next
        //
        if (get_sub_group_local_id() == SKC_PATHS_RECLAIM_SUBGROUP_SIZE - 1)
          {
            skc_block_id_t const next = SKC_CONCAT(n,SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST);

            SKC_CONCAT(n,SKC_PATHS_RECLAIM_BLOCK_EXPAND_I_LAST) = id;

            id = next;
          }

        //
        // count reclaimable blocks in each lane
        //
        SKC_PATHS_RECLAIM_PACKED_COUNT_DECLARE packed_count = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
        packed_count |= SKC_PATHS_RECLAIM_PACKED_COUNT_IS_BLOCK(n##I,I);

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        //
        // scan to find index of each block
        //
        SKC_PATHS_RECLAIM_PACKED_COUNT_DECLARE packed_index = ( 0 );

        SKC_PATHS_RECLAIM_PACKED_COUNT_SCAN_EXCLUSIVE_ADD(packed_index,packed_count);

        //
        // store blocks back to ring
        //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R) {                                       \
          skc_uint const index      = SKC_PATHS_RECLAIM_PACKED_COUNT_GET(packed_index,I); \
          skc_uint const count      = SKC_PATHS_RECLAIM_PACKED_COUNT_GET(packed_count,I); \
          skc_uint const bp_ids_idx = (bp_ids_base + index) & bp_mask;  \
          if (count > 0) {                                              \
            bp_ids[bp_ids_idx] = n##I;                                  \
          }                                                             \
          skc_uint const total = index + count;                         \
          bp_ids_base += sub_group_broadcast(total,SKC_PATHS_RECLAIM_SUBGROUP_SIZE-1); \
        }

        SKC_PATHS_RECLAIM_BLOCK_EXPAND();

        // printf("P %7u ! %u\n",bp_ids_idx,n##I);

        // any more nodes?
      } while (--count_nodes > 0);
    }
}

//
//
//
