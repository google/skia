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

#include "path.h"
#include "block_pool_cl.h"
#include "path_builder_cl_12.h"
#include "kernel_cl_12.h"

//
//
//

#if 0

//
// SIMD AVX2
//

#define SKC_PATHS_COPY_WORDS_PER_ELEM          8
#define SKC_PATHS_COPY_SUBGROUP_SIZE           1
#define SKC_PATHS_COPY_KERNEL_ATTRIBUTES

typedef skc_uint8  skc_paths_copy_elem;
typedef skc_uint8  skc_pb_idx_v;

#define SKC_PATHS_COPY_ELEM_EXPAND()           SKC_EXPAND_8()

#define SKC_IS_NOT_PATH_HEAD(sg,I)             ((sg) + I >= SKC_PATH_HEAD_WORDS)

#endif

//
//
//

#define SKC_PATHS_COPY_SUBGROUP_SIZE_MASK      (SKC_PATHS_COPY_SUBGROUP_SIZE - 1)
#define SKC_PATHS_COPY_ELEMS_PER_BLOCK         (SKC_DEVICE_BLOCK_WORDS / SKC_PATHS_COPY_ELEM_WORDS)
#define SKC_PATHS_COPY_ELEMS_PER_SUBBLOCK      (SKC_DEVICE_SUBBLOCK_WORDS / SKC_PATHS_COPY_ELEM_WORDS)
#define SKC_PATHS_COPY_ELEMS_PER_THREAD        (SKC_PATHS_COPY_ELEMS_PER_BLOCK / SKC_PATHS_COPY_SUBGROUP_SIZE)

// FIXME -- use SUBGROUP terminology everywhere
#define SKC_PATHS_COPY_SUBGROUP_WORDS          (SKC_PATHS_COPY_SUBGROUP_SIZE * SKC_PATHS_COPY_ELEM_WORDS)

//
//
//

#define SKC_PATHS_COPY_ELEMS_BEFORE_HEADER                              \
  (SKC_PATHS_COPY_SUBGROUP_SIZE * ((SKC_PATH_HEAD_WORDS / SKC_PATHS_COPY_ELEM_WORDS) / SKC_PATHS_COPY_SUBGROUP_WORDS))

#define SKC_PATHS_COPY_ELEMS_INCLUDING_HEADER                           \
  (SKC_PATHS_COPY_SUBGROUP_SIZE * ((SKC_PATH_HEAD_WORDS + SKC_PATHS_COPY_SUBGROUP_WORDS - 1) / SKC_PATHS_COPY_SUBGROUP_WORDS))

// #define SKC_PATHS_COPY_HEAD_ELEMS    ((SKC_PATH_HEAD_WORDS + SKC_PATHS_COPY_ELEM_WORDS - 1) / SKC_PATHS_COPY_ELEM_WORDS)

//
//
//

//
// BIT-FIELD EXTRACT/INSERT ARE NOT AVAILABLE IN OPENCL
//

#define SKC_CMD_PATHS_COPY_ONE_BITS              (SKC_TAGGED_BLOCK_ID_BITS_TAG + SKC_DEVICE_SUBBLOCK_WORDS_LOG2)

#define SKC_CMD_PATHS_COPY_ONE_MASK              SKC_BITS_TO_MASK(SKC_CMD_PATHS_COPY_ONE_BITS)

#define SKC_CMD_PATHS_COPY_ONE                   (1u << SKC_CMD_PATHS_COPY_ONE_BITS)

#define SKC_CMD_PATHS_COPY_GET_TAG(ti)           SKC_TAGGED_BLOCK_ID_GET_TAG(ti)

#define SKC_CMD_PATHS_COPY_GET_ROLLING(ti)       ((ti) >> SKC_CMD_PATHS_COPY_ONE_BITS)

#define SKC_CMD_PATHS_COPY_UPDATE_ROLLING(ti,b)  (((ti) & SKC_CMD_PATHS_COPY_ONE_MASK) | ((b) << SKC_TAGGED_BLOCK_ID_BITS_TAG))

//
//
//

skc_uint
skc_sub_group_local_id()
{
#if SKC_PATHS_COPY_SUBGROUP_SIZE > 1
  return get_sub_group_local_id();
#else
  return 0;
#endif
}

//
// convert an atomic read counter offset to a block id
//

skc_block_id_t
skc_bp_off_to_id(__global skc_block_id_t const * const bp_ids,
                 skc_uint                        const bp_idx_mask,
                 skc_uint                        const bp_reads,
                 skc_uint                        const bp_off)
{
  skc_uint const bp_idx = (bp_reads + bp_off) & bp_idx_mask;

  return bp_ids[bp_idx];
}

//
//
//

void
skc_copy_segs(__global skc_paths_copy_elem       * const bp_elems, // to
              skc_uint                             const bp_elems_idx,
              __global skc_paths_copy_elem const * const pb_elems, // from
              skc_uint                             const pb_elems_idx)
{
  for (skc_uint ii=0; ii<SKC_PATHS_COPY_ELEMS_PER_BLOCK; ii+=SKC_PATHS_COPY_SUBGROUP_SIZE)
    {
      (bp_elems+bp_elems_idx)[ii] = (pb_elems+pb_elems_idx)[ii];
    }

#if 0
  //
  // NOTE THIS IS PRINTING 8 ROWS
  //
  printf("%5u : (%8u) : { { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",
         (skc_uint)get_global_id(0),pb_elems_idx,
         as_float((pb_elems+pb_elems_idx)[0*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[1*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[2*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[3*SKC_PATHS_COPY_SUBGROUP_SIZE]));
  printf("%5u : (%8u) : { { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",
         (skc_uint)get_global_id(0),pb_elems_idx,
         as_float((pb_elems+pb_elems_idx)[4*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[5*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[6*SKC_PATHS_COPY_SUBGROUP_SIZE]),
         as_float((pb_elems+pb_elems_idx)[7*SKC_PATHS_COPY_SUBGROUP_SIZE]));
#endif
}

//
//
//

void
skc_copy_node(__global skc_paths_copy_elem       * const bp_elems, // to
              skc_uint                             const bp_elems_idx,
              __global skc_block_id_t      const * const bp_ids,
              skc_uint                             const bp_reads,
              skc_uint                             const bp_idx_mask,
              __global skc_paths_copy_elem const * const pb_elems, // from
              skc_uint                             const pb_elems_idx,
              skc_uint                             const pb_rolling)
{
  //
  // remap block id tags bp_elems the host-side rolling counter pb_elems a
  // device-side block pool id
  //
  for (skc_uint ii=0; ii<SKC_PATHS_COPY_ELEMS_PER_BLOCK; ii+=SKC_PATHS_COPY_SUBGROUP_SIZE)
    {
      // load block_id_tag words
      skc_paths_copy_elem elem   = (pb_elems + pb_elems_idx)[ii];

      // calculate ahead of time -- if elem was invalid then bp_idx is definitely invalid
      skc_pb_idx_v  const bp_idx = (bp_reads + SKC_CMD_PATHS_COPY_GET_ROLLING(elem - pb_rolling)) & bp_idx_mask;

      // FIXME ^^^^^ THE IDX PROBABLY DOESN'T NEED TO BE SHIFTED TWICE AND WE CAN SAVE A FEW INSTRUCTIONS

      //
      // FIXME -- SIMD can be fully parallelized since a bp_ids[] load
      // will _always_ be safe as long as we don't use the loaded
      // value!  So... fix UPDATE_ROLLING to be SIMD-friendly instead
      // of iterating over the vector components.
      //

      // only convert if original elem is not invalid

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                 \
      if (elem C != SKC_TAGGED_BLOCK_ID_INVALID) {              \
        skc_block_id_t const b = bp_ids[bp_idx C];              \
        elem C = SKC_CMD_PATHS_COPY_UPDATE_ROLLING(elem C,b);   \
      }

      // printf("%2u: < %8X, %8X, %8X >\n",ii,bp_idx,b,elem C);
      
      SKC_PATHS_COPY_ELEM_EXPAND();

      // store the elem back
      (bp_elems+bp_elems_idx)[ii] = elem;
    }
}

//
//
//

void
skc_host_map_update(__global skc_uint * const host_map,
                    skc_uint            const block,
                    skc_paths_copy_elem const elem)
{
  //
  // write first elem to map -- FIXME -- this is a little nasty
  // because it relies on the the host handle always being the first
  // word in the path header.
  //
  // OTOH, this is not unreasonable.  The alternative is to have a
  // separate kernel initializing the map.
  //
#if SKC_PATHS_COPY_SUBGROUP_SIZE > 1
  if (get_sub_group_local_id() == SKC_PATH_HEAD_OFFSET_HANDLE)
#endif
    {
#if SKC_PATHS_COPY_ELEM_WORDS == 1
      host_map[elem] = block; 
#if 0
      printf("[%u] = %u\n",elem,block);
#endif
#else
      host_map[elem.SKC_CONCAT(s,SKC_PATH_HEAD_OFFSET_HANDLE)] = block;
#endif
    }
}

//
//
//

void
skc_copy_head(__global skc_uint                  * const host_map,
              skc_uint                             const block,
              __global skc_paths_copy_elem       * const bp_elems, // to
              skc_uint                             const bp_elems_idx,
              __global skc_block_id_t      const * const bp_ids,
              skc_uint                             const bp_reads,
              skc_uint                             const bp_idx_mask,
              __global skc_paths_copy_elem const * const pb_elems, // from
              skc_uint                             const pb_elems_idx,
              skc_uint                             const pb_rolling)
{
  //
  // if there are more path header words than there are
  // threads-per-block then we can just copy the initial header words
  //
#if ( SKC_PATHS_COPY_ELEMS_BEFORE_HEADER > 0 )
  for (skc_uint ii=0; ii<SKC_PATHS_COPY_ELEMS_BEFORE_HEADER; ii+=SKC_PATHS_COPY_SUBGROUP_SIZE)
    {
      skc_paths_copy_elem const elem = (pb_elems+pb_elems_idx)[ii];

      (bp_elems+bp_elems_idx)[ii] = elem;

      if (ii == 0) {
        skc_host_map_update(host_map,block,elem);
      }
    }
#endif

  //
  // this is similar to copy node but the first H words of the path
  // header are not modified and simply copied
  //
  for (skc_uint ii=SKC_PATHS_COPY_ELEMS_BEFORE_HEADER; ii<SKC_PATHS_COPY_ELEMS_INCLUDING_HEADER; ii+=SKC_PATHS_COPY_SUBGROUP_SIZE)
    {
      skc_paths_copy_elem elem = (pb_elems+pb_elems_idx)[ii];

#if ( SKC_PATHS_COPY_ELEMS_BEFORE_HEADER == 0 )
      if (ii == 0) {
        skc_host_map_update(host_map,block,elem);
      }
#endif
      // calculate ahead of time -- if elem was invalid then bp_idx is definitely invalid
      skc_pb_idx_v const bp_idx = (bp_reads + SKC_CMD_PATHS_COPY_GET_ROLLING(elem - pb_rolling)) & bp_idx_mask;

      //
      // FIXME -- SIMD can be fully parallelized since a bp_ids[] load
      // will _always_ be safe as long as we don't use the loaded
      // value!  So... fix UPDATE_ROLLING to be SIMD-friendly instead
      // of iterating over the vector components.
      //

      // FIXME ^^^^^ THE IDX PROBABLY DOESN'T NEED TO BE SHIFTED TWICE AND WE CAN SAVE A FEW INSTRUCTIONS

      // FIXME -- MIX MIX MIX MIX / SELECT

      // only convert if original elem is not invalid
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                         \
      if (SKC_IS_NOT_PATH_HEAD(ii,I) && (elem C != SKC_TAGGED_BLOCK_ID_INVALID)) { \
        skc_block_id_t const b = bp_ids[bp_idx C];                      \
        elem C = SKC_CMD_PATHS_COPY_UPDATE_ROLLING(elem C,b);           \
      }

      // printf("%2u: ( %8X, %8X, %8X )\n",ii,bp_idx,b,elem C);

      SKC_PATHS_COPY_ELEM_EXPAND();

      // store the elem back
      (bp_elems+bp_elems_idx)[ii] = elem;
    }

  //
  // the remaining words are treated like a node
  //
  for (skc_uint ii=SKC_PATHS_COPY_ELEMS_INCLUDING_HEADER; ii<SKC_PATHS_COPY_ELEMS_PER_BLOCK; ii+=SKC_PATHS_COPY_SUBGROUP_SIZE)
    {
      // load block_id_tag words
      skc_paths_copy_elem elem   = (pb_elems+pb_elems_idx)[ii];

      // calculate ahead of time
      skc_pb_idx_v  const bp_idx = (bp_reads + SKC_CMD_PATHS_COPY_GET_ROLLING(elem - pb_rolling)) & bp_idx_mask;

      //
      // FIXME -- SIMD can be fully parallelized since a bp_ids[] load
      // will _always_ be safe as long as we don't use the loaded
      // value!  So... fix UPDATE_ROLLING to be SIMD-friendly instead
      // of iterating over the vector components.
      //

      // FIXME ^^^^^ THE IDX PROBABLY DOESN'T NEED TO BE SHIFTED TWICE AND WE CAN SAVE A FEW INSTRUCTIONS

      // only convert if original elem is not invalid
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,R)                                 \
      if (elem C != SKC_TAGGED_BLOCK_ID_INVALID) {              \
        skc_block_id_t const b = bp_ids[bp_idx C];              \
        elem C = SKC_CMD_PATHS_COPY_UPDATE_ROLLING(elem C,b);   \
      }

      // printf("%2u: [ %8X, %8X, %8X ]\n",ii,bp_idx,b,elem C);

      SKC_PATHS_COPY_ELEM_EXPAND();

      // store the elem
      (bp_elems+bp_elems_idx)[ii] = elem;
    }
}

//
// FIXME -- pack some of these constant integer args in a vec or struct
//

__kernel
SKC_PATHS_COPY_KERNEL_ATTRIBS
void
skc_kernel_paths_copy
(__global skc_uint                        * const host_map,

 __global skc_block_id_t            const * const bp_ids,
 __global skc_paths_copy_elem             * const bp_elems,
 skc_uint                                   const bp_idx_mask, // pow2 modulo mask for block pool ring

 __global skc_uint                  const * const bp_alloc,    // block pool ring base
 skc_uint                                   const bp_alloc_idx,// which subbuf

 __global union skc_tagged_block_id const * const pb_cmds,
 __global skc_paths_copy_elem       const * const pb_elems,

 skc_uint                                   const pb_size,     // # of commands/blocks in buffer
 skc_uint                                   const pb_rolling,  // shifted rolling counter base

 skc_uint                                   const pb_prev_from,
 skc_uint                                   const pb_prev_span,
 skc_uint                                   const pb_curr_from)
{
  //
  // THERE ARE 3 TYPES OF PATH COPYING COMMANDS:
  //
  // - HEAD
  // - NODE
  // - SEGS
  //
  // THESE ARE SUBGROUP ORIENTED KERNELS
  //
  // A SUBGROUP CAN OPERATE ON [1,N] BLOCKS
  //

  //
  // It's likely that peak bandwidth is achievable with a single
  // workgroup.
  //
  // So let's keep the grids modestly sized and for simplicity and
  // portability, let's assume that a single workgroup can perform all
  // steps in the copy.
  //
  // Launch as large of a workgroup as possiblex
  //
  // 1. ATOMICALLY ALLOCATE BLOCKS BP_ELEMS POOL
  // 2. CONVERT COMMANDS IN PB_ELEMS BLOCK OFFSETS
  // 3. FOR EACH COMMAND:
  //      - HEAD: SAVED HEAD ID PB_ELEMS MAP. CONVERT AND COPY H INDICES.
  //      - NODE: CONVERT AND COPY B INDICES
  //      - SEGS: BULK COPY
  //
  // B : number of words in block -- always pow2
  // W : intelligently/arbitrarily chosen factor of B -- always pow2
  //

  //
  // There are several approaches to processing the commands:
  //
  // 1. B threads are responsible for one block. All threads broadcast
  //    load a single command word. Workgroup size must be a facpb_elemsr of
  //    B.
  //
  // 2. W threads process an entire block. W will typically be the
  //    device's subgroup/warp/wave width. W threads broadcast load a
  //    single command word.
  //
  // 3. W threads process W blocks. W threads load W command words and
  //    process W blocks.
  //
  // Clearly (1) has low I/O intensity but will achieve high
  // parallelism by activating the most possible threads. The downside
  // of this kind of approach is that the kernel will occupy even a
  // large GPU with low intensity work and reduce opportunities for
  // concurrent kernel execution (of other kernels).
  //
  // See Vasily Volkov's CUDA presentation describing these tradeoffs.
  //
  // Note that there are many other approaches.  For example, similar
  // pb_elems (1) but each thread loads a pow2 vector of block data.
  //

  // load the copied atomic read "base" from gmem
  skc_uint const bp_reads = bp_alloc[bp_alloc_idx];
  // will always be less than 2^32
  skc_uint const gid      = get_global_id(0);
  // every subgroup/simd that will work on the block loads the same command
  skc_uint const sg_idx   = gid / SKC_PATHS_COPY_SUBGROUP_SIZE;
  // path builder data can be spread across two spans
  skc_uint       pb_idx   = sg_idx + ((sg_idx < pb_prev_span) ? pb_prev_from : pb_curr_from);

  // no need pb_elems make this branchless
  if (pb_idx >= pb_size)
    pb_idx -= pb_size;

  // broadcast load the command
  union skc_tagged_block_id const pb_cmd       = pb_cmds[pb_idx];

  // what do we want pb_elems do with this block?
  skc_cmd_paths_copy_tag    const tag          = SKC_CMD_PATHS_COPY_GET_TAG(pb_cmd.u32);

  // compute offset from rolling base to get index into block pool ring allocation
  skc_uint                  const bp_off       = SKC_CMD_PATHS_COPY_GET_ROLLING(pb_cmd.u32 - pb_rolling);

  // convert the pb_cmd's offset counter pb_elems a block id
  skc_block_id_t            const block        = skc_bp_off_to_id(bp_ids,bp_idx_mask,bp_reads,bp_off);

#if 0
  if (get_sub_group_local_id() == 0) {
    printf("bp_off/reads = %u / %u\n",bp_off,bp_reads);
    printf("< %8u >\n",block);
  }
#endif

  // FIXME -- could make this 0 for SIMD, gid&mask or get_sub_group_local_id()
  skc_uint                 const tid          = gid & SKC_PATHS_COPY_SUBGROUP_SIZE_MASK;

  // calculate bp_elems (to) / pb_elems (from)
  skc_uint                 const bp_elems_idx = block  * SKC_PATHS_COPY_ELEMS_PER_SUBBLOCK + tid;
  skc_uint                 const pb_elems_idx = pb_idx * SKC_PATHS_COPY_ELEMS_PER_BLOCK    + tid;

  if      (tag == SKC_CMD_PATHS_COPY_TAG_SEGS)
    {
#if 0
      if (tid == 0)
        printf("%3u, segs\n",bp_off);
#endif
      skc_copy_segs(bp_elems,
                    bp_elems_idx,
                    pb_elems,
                    pb_elems_idx);
    }
  else if (tag == SKC_CMD_PATHS_COPY_TAG_NODE)
    {
#if 0
      if (tid == 0)
        printf("%3u, NODE\n",bp_off);
#endif
      skc_copy_node(bp_elems, // to
                    bp_elems_idx,
                    bp_ids,
                    bp_reads,
                    bp_idx_mask,
                    pb_elems, // from
                    pb_elems_idx,
                    pb_rolling);
    }
  else // ( tag == SKC_CMD_PATHS_COPY_TAG_HEAD)
    {
#if 0
      if (tid == 0)
        printf("%3u, HEAD\n",bp_off);
#endif
      skc_copy_head(host_map,
                    block,
                    bp_elems, // to
                    bp_elems_idx,
                    bp_ids,
                    bp_reads,
                    bp_idx_mask,
                    pb_elems, // from
                    pb_elems_idx,
                    pb_rolling);
    }
}

//
//
//

__kernel
SKC_PATHS_ALLOC_KERNEL_ATTRIBS
void
skc_kernel_paths_alloc(__global skc_uint volatile * const bp_atomics,
                       __global skc_uint          * const bp_alloc,
                       skc_uint                     const bp_alloc_idx,
                       skc_uint                     const pb_cmd_count)
{
  //
  // allocate blocks in block pool
  //
  skc_uint const reads = atomic_add(bp_atomics+SKC_BP_ATOMIC_OFFSET_READS,pb_cmd_count);

  // store in slot
  bp_alloc[bp_alloc_idx] = reads;

#if 0
  printf("pc: %8u + %u\n",reads,pb_cmd_count);
#endif
}

//
//
//
