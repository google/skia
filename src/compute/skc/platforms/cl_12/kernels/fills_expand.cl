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

#include "block.h"
#include "path.h"
#include "common.h"
#include "atomic_cl.h"
#include "raster_builder_cl_12.h"
#include "kernel_cl_12.h"

//
//
//

#define SKC_FILLS_EXPAND_SUBGROUP_SIZE_MASK (SKC_FILLS_EXPAND_SUBGROUP_SIZE - 1)

#define SKC_FILLS_EXPAND_ELEMS_PER_BLOCK    (SKC_DEVICE_BLOCK_WORDS    / SKC_FILLS_EXPAND_ELEM_WORDS)
#define SKC_FILLS_EXPAND_ELEMS_PER_SUBBLOCK (SKC_DEVICE_SUBBLOCK_WORDS / SKC_FILLS_EXPAND_ELEM_WORDS)

#define SKC_FILLS_EXPAND_ELEMS_PER_THREAD   (SKC_FILLS_EXPAND_ELEMS_PER_BLOCK / SKC_FILLS_EXPAND_SUBGROUP_SIZE)

//
//
//

#define SKC_FILLS_EXPAND_X  (SKC_DEVICE_BLOCK_WORDS / SKC_FILLS_EXPAND_SUBGROUP_SIZE)

//
//
//

#if   ( SKC_FILLS_EXPAND_X == 1 )
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND()       SKC_EXPAND_1()
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST  0

#elif ( SKC_FILLS_EXPAND_X == 2 )
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND()       SKC_EXPAND_2()
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST  1

#elif ( SKC_FILLS_EXPAND_X == 4 )
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND()       SKC_EXPAND_4()
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST  3

#elif ( SKC_FILLS_EXPAND_X == 8 )
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND()       SKC_EXPAND_8()
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST  7

#elif ( SKC_FILLS_EXPAND_X == 16)
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND()       SKC_EXPAND_16()
#define SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST  15

#else
#error "MISSING SKC_FILLS_EXPAND_X"
#endif

//
// Fill and rasterize cmds only differ in their first word semantics
//

union skc_cmd_expand
{
  union skc_cmd_fill      fill;
  union skc_cmd_rasterize rasterize;
};

//
//
//

union skc_path_elem
{
  skc_uint  u32;
  skc_float f32;
};

//
// COMPILE-TIME AND RUN-TIME MACROS
//

#define SKC_ELEM_IN_RANGE(X,I)                                          \
  (skc_bool)SKC_GTE_MACRO(X,(I ) * SKC_FILLS_EXPAND_SUBGROUP_SIZE) &&   \
  (skc_bool)SKC_LT_MACRO(X,(I+1) * SKC_FILLS_EXPAND_SUBGROUP_SIZE)

#define SKC_ELEM_GTE(X,I)                                       \
  SKC_GTE_MACRO(X,(I+1) * SKC_FILLS_EXPAND_SUBGROUP_SIZE)

//
// FIXME -- slate these for replacement
//

#define SKC_BROADCAST(E,S,I)                                            \
  sub_group_broadcast(E##I.u32,S - I * SKC_FILLS_EXPAND_SUBGROUP_SIZE)

#define SKC_BROADCAST_LAST_HELPER(E,I)                                  \
  sub_group_broadcast(E##I.u32,SKC_FILLS_EXPAND_SUBGROUP_SIZE - 1)

#define SKC_BROADCAST_LAST(E,I)                 \
  SKC_BROADCAST_LAST_HELPER(E,I)

//
//
//

void
skc_cmds_out_append(__global union skc_cmd_rasterize * const cmds_out,
                    skc_uint                         * const out_idx,
                    union skc_cmd_expand             * const cmd,
                    union skc_path_elem                const e,
                    skc_uint                           const e_idx)
{
  //
  // FIXME -- we can append a large number of nodeword indices to a
  // local SMEM queue and flush when full.  It may or may not be a
  // performance win on some architectures.
  //
  skc_bool const is_elem = SKC_TAGGED_BLOCK_ID_GET_TAG(e.u32) < SKC_BLOCK_ID_TAG_PATH_NEXT;
  skc_uint const offset  = sub_group_scan_inclusive_add(is_elem ? 1 : 0);

  cmd->rasterize.nodeword = e_idx;

  if (is_elem) {
    cmds_out[*out_idx + offset] = cmd->rasterize;
  }

  *out_idx += sub_group_broadcast(offset,SKC_FILLS_EXPAND_SUBGROUP_SIZE-1);
}

//
//
//

__kernel
SKC_FILLS_EXPAND_KERNEL_ATTRIBS
void
skc_kernel_fills_expand(__global union skc_path_elem     const    * const blocks,
                        __global skc_uint                volatile * const atomics,
                        __global skc_block_id_t          const    * const map,
                        __global union skc_cmd_fill      const    * const cmds_in,
                        __global union skc_cmd_rasterize          * const cmds_out)
{
  //
  // Need to harmonize the way we determine a subgroup's id.  In this
  // kernel it's not as important because no local memory is being
  // used.  Although the device/mask calc to determine subgroup and
  // lanes is still proper, we might want to make it clearer that
  // we're working with subgroups by using the subgroup API.
  //
  // every subgroup/simd that will work on the block loads the same command
  //
#if (__OPENCL_VERSION__ < 200)
  skc_uint const       cmd_stride = get_num_sub_groups();
#else
  skc_uint const       cmd_stride = get_enqueued_num_sub_groups(); // 2.0 supports non-uniform workgroups
#endif
  skc_uint             cmd_idx    = get_group_id(0) * cmd_stride + get_sub_group_id();

  // load fill command -- we reuse y component
  union skc_cmd_expand cmd        = { .fill = cmds_in[cmd_idx] };

  // get the path header block from the map
  skc_block_id_t       id         = map[cmd.fill.path];

#if 0
  if (get_sub_group_local_id() == 0)
    printf("expand[%u] = %u\n",cmd_idx,id);
#endif

  //
  // blindly load all of the head elements into registers
  //
  skc_uint head_idx = id * SKC_FILLS_EXPAND_ELEMS_PER_SUBBLOCK + get_sub_group_local_id();

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
  union skc_path_elem h##I = blocks[head_idx + I * SKC_FILLS_EXPAND_SUBGROUP_SIZE];

  SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

  //
  // pick out count.nodes and count.prims from the header
  //
  skc_uint count_nodes, count_prims;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
  if (SKC_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_NODES,I)) {                \
    count_nodes  = SKC_BROADCAST(h,SKC_PATH_HEAD_OFFSET_NODES,I);       \
  }                                                                     \
  if (SKC_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_PRIMS,I)) {                \
    count_prims  = SKC_BROADCAST(h,SKC_PATH_HEAD_OFFSET_PRIMS,I);       \
  }

  SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

  //
  // debug of path head
  //
#if 0
  skc_uint count_blocks;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
  if (SKC_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_BLOCKS,I)) {               \
    count_blocks = SKC_BROADCAST(h,SKC_PATH_HEAD_OFFSET_BLOCKS,I);      \
  }

  SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

  if (get_sub_group_local_id() == 0)
    printf("path header = { %5u, %5u, %5u }\n",
           count_blocks,count_nodes,count_prims);
#endif

  //
  // acquire slots in the expanded cmd extent
  //
  // decrement prim_idx by 1 so we can use inclusive warp scan later
  //
  skc_uint out_idx = 0;

  if (get_sub_group_local_id() == 0) {
    out_idx = SKC_ATOMIC_ADD_GLOBAL_RELAXED_SUBGROUP
      (atomics+SKC_RASTER_COHORT_ATOMIC_OFFSET_CMDS,count_prims) - 1;
  }

  out_idx = sub_group_broadcast(out_idx,0);

  //
  // process ids trailing the path header
  //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
  if (!SKC_ELEM_GTE(SKC_PATH_HEAD_OFFSET_IDS,I)) {                      \
    if (SKC_ELEM_IN_RANGE(SKC_PATH_HEAD_OFFSET_IDS,I)) {                \
      if (get_sub_group_local_id() + I * SKC_FILLS_EXPAND_SUBGROUP_SIZE < SKC_PATH_HEAD_OFFSET_IDS) { \
        h##I.u32 = SKC_TAGGED_BLOCK_ID_INVALID;                         \
      }                                                                 \
    }                                                                   \
    skc_cmds_out_append(cmds_out,&out_idx,&cmd,h##I,                    \
                        head_idx + I * SKC_FILLS_EXPAND_SUBGROUP_SIZE); \
  }

  SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

  //
  // we're done if it was just the header
  //
  if (count_nodes == 0)
    return;

  //
  // otherwise, process the nodes
  //

  //
  // get id of next node
  //
  id = SKC_TAGGED_BLOCK_ID_GET_ID(SKC_BROADCAST_LAST(h,SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST));

  //
  // the following blocks are nodes
  //
  while (true)
    {
      // get index of each element
      skc_uint node_idx = id * SKC_FILLS_EXPAND_ELEMS_PER_SUBBLOCK + get_sub_group_local_id();

      //
      // blindly load all of the node elements into registers
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      union skc_path_elem const n##I = blocks[node_idx + I * SKC_FILLS_EXPAND_SUBGROUP_SIZE];

      SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

      //
      // append all valid ids
      //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      skc_cmds_out_append(cmds_out,&out_idx,&cmd,n##I,                  \
                          node_idx + I * SKC_FILLS_EXPAND_SUBGROUP_SIZE);

      SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND();

      // any more nodes?
      if (--count_nodes == 0)
        return;

      //
      // get id of next node
      //
      id = SKC_TAGGED_BLOCK_ID_GET_ID(SKC_BROADCAST_LAST(n,SKC_FILLS_EXPAND_PATH_BLOCK_EXPAND_I_LAST));
    }
}

//
//
//
