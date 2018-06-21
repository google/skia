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
#include "raster.h"
#include "atomic_cl.h"
#include "block_pool_cl.h"
#include "raster_builder_cl_12.h"
#include "kernel_cl_12.h"

//
// There is a fixed-size meta table per raster cohort that we use to
// peform a mostly coalesced sizing and allocation of blocks.
//
// This code is simple and fast.
//

__kernel
SKC_RASTERS_ALLOC_KERNEL_ATTRIBS
void
skc_kernel_rasters_alloc(__global SKC_ATOMIC_UINT volatile * const bp_atomics,
                         __global skc_block_id_t  const    * const bp_ids,
                         skc_uint                            const bp_mask, // pow2 modulo mask for block pool ring
                         __global skc_block_id_t           * const map,
                         __global skc_uint                 * const metas,
                         __global skc_uint        const    * const raster_ids, // FIXME -- CONSTANT
                         skc_uint                            const count)
{
  // access to the meta extent is linear
  skc_uint const gid       = get_global_id(0);
  skc_bool const is_active = gid < count;

  //
  // init with defaults for all lanes
  //
  union skc_raster_cohort_meta_inout meta         = { .in.u32v4 = { 0, 0, 0, 0 } };
  skc_uint                           raster_id    = SKC_UINT_MAX;
  skc_uint                           extra_blocks = 0;

  if (is_active)
    {
      // load meta_in
      meta.in.u32v4     = vload4(gid,metas);

      // load raster_id as early as possible
      raster_id         = raster_ids[gid];

#if 0
      printf("%3u + %5u, %5u, %5u, %5u\n",
             gid,
             meta.in.blocks,
             meta.in.offset,
             meta.in.pk,
             meta.in.rk);
#endif

      // how many blocks will the ttpb blocks consume?
      extra_blocks      = ((meta.in.pk * SKC_TILE_RATIO + SKC_DEVICE_SUBBLOCKS_PER_BLOCK - SKC_TILE_RATIO) / 
                           SKC_DEVICE_SUBBLOCKS_PER_BLOCK);

      // total keys
      meta.out.keys    += meta.in.pk;

      // how many blocks do we need to store the keys in the head and trailing nodes?
      skc_uint const hn = ((SKC_RASTER_HEAD_DWORDS + meta.out.keys + SKC_RASTER_NODE_DWORDS - 2) /
                           (SKC_RASTER_NODE_DWORDS - 1));
      // increment blocks
      extra_blocks     += hn;

      // how many nodes trail the head?
      meta.out.nodes    = hn - 1;
      
      // update blocks
      meta.out.blocks  += extra_blocks;

#if 0
      printf("%3u - %5u, %5u, %5u, %5u\n",
             gid,
             meta.out.blocks,
             meta.out.offset,
             meta.out.nodes,
             meta.out.keys);
#endif
    }

  //
  // allocate blocks from block pool
  //
  // first perform a prefix sum on the subgroup to reduce atomic
  // operation traffic
  //
  // note this idiom can be implemented with vectors, subgroups or
  // workgroups
  //
  
  skc_uint const prefix = SKC_RASTERS_ALLOC_INCLUSIVE_ADD(extra_blocks);
  skc_uint       reads  = 0;

  // last lane performs the block pool allocation with an atomic increment
  if (SKC_RASTERS_ALLOC_LOCAL_ID() == SKC_RASTERS_ALLOC_GROUP_SIZE - 1) {
    reads = SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(bp_atomics+SKC_BP_ATOMIC_OFFSET_READS,prefix); // ring_reads
  }

  // broadcast block pool base to all lanes
  reads = SKC_RASTERS_ALLOC_BROADCAST(reads,SKC_RASTERS_ALLOC_GROUP_SIZE - 1);

  // update base for each lane
  reads += prefix - extra_blocks;

  //
  // store meta header
  //
  if (is_active)
    {
      // store headers back to meta extent
      vstore4(meta.out.u32v4,gid,metas);

      // store reads
      metas[SKC_RASTER_COHORT_META_OFFSET_READS + gid] = reads; 

      // get block_id of each raster head 
      skc_block_id_t const block_id = bp_ids[reads & bp_mask];

      // update map
      map[raster_id] = block_id;

#if 0
      printf("alloc: %u / %u\n",raster_id,block_id);
#endif
    }
}

//
//
//
