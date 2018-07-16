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

// get rid of these
#include <stdio.h>
#include <stdlib.h>

//
//
//

#include "hs/cl/hs_cl_launcher.h"

#include "common/cl/assert_cl.h"

#include "context.h"
#include "grid.h"
#include "raster.h"
#include "extent_ring.h"
#include "raster_builder.h"

#include "tile.h"

#include "config_cl.h"
#include "runtime_cl_12.h"
#include "extent_cl_12.h"
#include "raster_builder_cl_12.h"

//
// RASTERIZATION SUB-PIPELINE
// --------------------------
//
// Phase 1: expand commands
//
// Phase 2: rasterize
//
// Phase 3: sort & segment || release paths
//
// Phase 4: prefix
//
// Phase 5: release rasters
//
//                                                      RASTER  COHORT
//                                                      ==============
//
//                      BUILDER                           RASTERIZER                              POST PROCESSING
//   <----------------------------------------------->  <------------>  <--------------------------------------------------------------------->
//
//   fill cmds  transforms  raster clips  path release  rasterize cmds  cohort map  raster release  TTSB  TTSK  cohort atomics  context atomics
//   ---------  ----------  ------------  ------------  --------------  ----------  --------------  ----  ----  --------------  ---------------
//      1,2        1,2           1,2           1,2             2            1-4         1,2,3,4       2-4   2-4       2-4            global
//
//
// NOTES: FINE-GRAINED SVM
// -----------------------
//
//   1) In a fine-grained system we know the exact number of
//      rasterize cmds per segment type before phase 1
//
//   2) A raster that's "under construction" shouldn't be rasterized
//      until it is complete.  This implies that a raster is not part
//      of a cohort until it is complete.  The raster builder must
//      handle raster promises being "forced" to completion -- this is
//      likely the result of composition construction and subsequent
//      rendering to a surface.
//
//   3) The raster cohort rasterizer state retains the fill cmd,
//      transform, raster clip and path release "ring" extents.
//
//   4) The rasterize cmd extent sizes (line, quad, cubic, rational
//      quad, rational cubic) are known ahead of time.
//
//   5) The raster cohort post processor is standalone and retains the
//      raster_map, cohort atomics, TTSK_RYX extent, and raster
//      references until complete.
//

//
// Notes:
//
// - Could have a pipeline stage before expansion count the exact
//   number of line/quad/cubic commands but the command buffers are
//   relatively small (64-bit commands * # of path segments).
//

//                          raster
//                          cohort atomics path_ids raster_ids transforms clips cmds_fill cmds_l/q/c ttsk_ryx
//
//
// BEGIN                      ^
//                            |
//   EXPAND                   |
//                            |
//   RASTERIZE                |
//                            |
//   SORT || RELEASE PATHS    |
//                            |
//   PREFIX                   |
//                            |
//   RELEASE RASTERS          |
//                            |
// END                        v
//
//
// BEGIN
//
//   EXPAND                   -- PRODUCES:   one or more extents of rasterization commands
//
//   RASTERIZE                -- DEPENDENCY: requires size of command extents before launching
//                            -- PRODUCES:   an extent of ttsk_ryx keys
//
//   SORT || RELEASE PATHS    -- DEPENDENCY: requires size of key extent before launching
//                            -- PRODUCES:   sorted array of keys
//
//   PREFIX                   -- DEPENDENCY: none -- can execute after SORT because grid size is number of rasters
//
//   RELEASE RASTERS          -- DEPENDENCY: none -- can execute after prefix
//
// END
//

// ------------------------
//
// DEPENDENCY is cleanly implemented with a host callback or device kernel launcher
//
// Can this hide resource acquisition?  Yes.  But there are two cases:
//
// 1. acqusition of resources occurs on the host thread and lack of
//    resources drains the host command queue until resources are
//    available (OpenCL 2.x)
//
// 2. the host commands lazily acquire resources (OpenCL 1.2)
//
// ------------------------
//
// How to express?
//
// Each substage launches its successors.  This supports both dependency models.
//
// If OpenCL 1.2 then the substage can't be launched until the prior
// stage's event is complete.  So this requires registering a callback
// to invoke the substage.
//
// ------------------------

//
// BUILD
//

struct skc_raster_builder_impl
{
  struct skc_raster_builder    * raster_builder;
  struct skc_runtime           * runtime;

  skc_grid_t                     cohort;

  // these are all durable/perm extents
  struct skc_extent_phrwg_thr1s  path_ids;    // read/write by host
  struct skc_extent_phw1g_tdrNs  transforms;  // write once by host + read by device
  struct skc_extent_phw1g_tdrNs  clips;       // write once by host + read by device
  struct skc_extent_phw1g_tdrNs  fill_cmds;   // write once by host + read by device
  struct skc_extent_phrwg_tdrNs  raster_ids;  // read/write by host + read by device

  struct {
    cl_kernel                    fills_expand;
    cl_kernel                    rasterize_all;
    cl_kernel                    segment;
    cl_kernel                    rasters_alloc;
    cl_kernel                    prefix;
  } kernels;
};

//
// RASTER COHORT
//
// This sub-pipeline snapshots the raster builder and then acquires
// and releases host and device resources as necessary (as late as
// possible).
//
// Note that the cohort extents are ephemeral and are only used by one
// or more stages of a the rasterization sub-pipeline.
//
// The pipeline implementation may vary between compute platforms.
//

struct skc_raster_cohort
{
  struct skc_raster_builder_impl    * impl;

  struct skc_extent_phrwg_thr1s_snap  path_ids;    // read/write by host
  struct skc_extent_phw1g_tdrNs_snap  transforms;  // write once by host + read by device
  struct skc_extent_phw1g_tdrNs_snap  clips;       // write once by host + read by device
  struct skc_extent_phw1g_tdrNs_snap  fill_cmds;   // write once by host + read by device
  struct skc_extent_phrwg_tdrNs_snap  raster_ids;  // read/write by host + read by device

  cl_command_queue                    cq;

  // sub-pipeline atomics
  struct skc_extent_thr_tdrw          atomics;

  // path primitives are expanded into line/quad/cubic/rational cmds
  struct skc_extent_tdrw              cmds;

  // rasterization output
  struct skc_extent_tdrw              keys;
  // struct skc_extent_thrw_tdrw      keys;

  // post-sort extent with metadata for each raster
  struct skc_extent_tdrw              metas;
  // struct skc_extent_thrw_tdrw      metas;

  // subbuf id
  skc_subbuf_id_t                     id;

  //
  // pipeline also uses the following global resources:
  //
  // - command queue from global factory
  // - global block pool and its atomics
  // - global path and raster host id map
  // - temporary host and device allocations
  //
};

//
// TTRK (64-BIT COMPARE)
//
//    0                                  63
//    | TTSB ID |   X  |   Y  | COHORT ID |
//    +---------+------+------+-----------+
//    |    27   |  12  |  12  |     13    |
//
//
// TTRK (32-BIT COMPARE)
//
//    0                                        63
//    | TTSB ID | N/A |   X  |   Y  | COHORT ID |
//    +---------+-----+------+------+-----------+
//    |    27   |  5  |  12  |  12  |     8     |
//

//
// TTRK is sortable intermediate key format for TTSK
//
// We're going to use the 32-bit comparison version for now
//

union skc_ttrk
{
  skc_ulong  u64;
  skc_uint2  u32v2;

  struct {
    skc_uint block    : SKC_TTXK_LO_BITS_ID;
    skc_uint na0      : SKC_TTRK_LO_BITS_NA;
    skc_uint x        : SKC_TTXK_HI_BITS_X;
    skc_uint y        : SKC_TTXK_HI_BITS_Y;
    skc_uint cohort   : SKC_TTRK_HI_BITS_COHORT;
  };

  struct {
    skc_uint na1;
    skc_uint yx       : SKC_TTXK_HI_BITS_YX;
    skc_uint na2      : SKC_TTRK_HI_BITS_COHORT;
  };

  struct {
    skc_uint na3;
    skc_uint na4      : SKC_TTXK_HI_BITS_X;
    skc_uint cohort_y : SKC_TTRK_HI_BITS_COHORT_Y;
  };
};

//
//
//

static
void
skc_raster_builder_pfn_release(struct skc_raster_builder_impl * const impl)
{
  // decrement reference count
  if (--impl->raster_builder->refcount != 0)
    return;

  //
  // otherwise, dispose of the the raster builder and its impl
  //
  struct skc_runtime * const runtime = impl->runtime;

  // free the raster builder
  skc_runtime_host_perm_free(runtime,impl->raster_builder);

  // free durable/perm extents
  skc_extent_phrwg_thr1s_free(runtime,&impl->path_ids);
  skc_extent_phw1g_tdrNs_free(runtime,&impl->transforms);
  skc_extent_phw1g_tdrNs_free(runtime,&impl->clips);
  skc_extent_phw1g_tdrNs_free(runtime,&impl->fill_cmds);
  skc_extent_phrwg_tdrNs_free(runtime,&impl->raster_ids);

  // release kernels
  cl(ReleaseKernel(impl->kernels.fills_expand));
  cl(ReleaseKernel(impl->kernels.rasterize_all));

#if 0
  cl(ReleaseKernel(impl->kernels.rasterize_lines));
  cl(ReleaseKernel(impl->kernels.rasterize_quads));
  cl(ReleaseKernel(impl->kernels.rasterize_cubics));
#endif

  cl(ReleaseKernel(impl->kernels.segment));
  cl(ReleaseKernel(impl->kernels.rasters_alloc));
  cl(ReleaseKernel(impl->kernels.prefix));

  // free the impl
  skc_runtime_host_perm_free(runtime,impl);
}

//
//
//

static
void
skc_raster_builder_rasters_release(struct skc_runtime * const runtime,
                                   skc_raster_t const * const rasters,
                                   skc_uint             const size,
                                   skc_uint             const from,
                                   skc_uint             const to)
{
  if (from <= to) // no wrap
    {
      skc_raster_t const * rasters_from = rasters + from;
      skc_uint             count_from   = to      - from;

      skc_grid_deps_unmap(runtime->deps,rasters_from,count_from);
      skc_runtime_raster_device_release(runtime,rasters_from,count_from);
    }
  else // from > to implies wrap
    {
      skc_raster_t const * rasters_lo = rasters + from;
      skc_uint             count_lo   = size    - from;

      skc_grid_deps_unmap(runtime->deps,rasters_lo,count_lo);
      skc_runtime_raster_device_release(runtime,rasters_lo,count_lo);

      skc_grid_deps_unmap(runtime->deps,rasters,to);
      skc_runtime_raster_device_release(runtime,rasters,to);
    }
}

static
void
skc_raster_builder_paths_release(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phrwg_thr1s_snap * const snap)
{
  // release lo
  skc_runtime_path_device_release(runtime,snap->hr1.lo,snap->count.lo);

  // release hi
  if (snap->count.hi)
    skc_runtime_path_device_release(runtime,snap->hr1.hi,snap->count.hi);
}

static
void
skc_raster_builder_cohort_grid_pfn_dispose(skc_grid_t const grid)
{
  //
  // ALLOCATED RESOURCES
  //
  // path_ids          -
  // raster_ids        a
  // transforms        -
  // clips             -
  // fill_cmds         -
  // cq                a
  // cohort atomics    a
  // cmds              -
  // keys              a
  // meta              a
  //

  struct skc_raster_cohort       * const cohort  = skc_grid_get_data(grid);
  struct skc_raster_builder_impl * const impl    = cohort->impl;
  struct skc_runtime             * const runtime = impl->runtime;

  //
  // release paths -- FIXME -- Note that releasing paths can be
  // performed after rasterization is complete
  //

  // snap alloc the paths -- this host snap simply sets up pointers
  skc_extent_phrwg_thr1s_snap_alloc(runtime,&impl->path_ids,&cohort->path_ids);

  // unmap and release raster ids
  skc_raster_builder_paths_release(runtime,&cohort->path_ids);

  // release path ids
  skc_extent_phrwg_thr1s_snap_free(runtime,&cohort->path_ids);

  //
  // release rasters
  //
  skc_uint const size = cohort->raster_ids.snap->ring->size.pow2;
  skc_uint const from = skc_extent_ring_snap_from(cohort->raster_ids.snap);
  skc_uint const to   = skc_extent_ring_snap_to(cohort->raster_ids.snap);

  // unmap and release raster ids
  skc_raster_builder_rasters_release(runtime,impl->raster_ids.hrw,size,from,to);

  // release cohort's remaining allocated resources
  skc_extent_phrwg_tdrNs_snap_free(runtime,&cohort->raster_ids);
  skc_runtime_release_cq_in_order(runtime,cohort->cq);
  skc_extent_thr_tdrw_free(runtime,&cohort->atomics);
  skc_extent_tdrw_free(runtime,&cohort->keys);
  skc_extent_tdrw_free(runtime,&cohort->metas);
  // skc_extent_thrw_tdrw_free(runtime,&cohort->keys);
  // skc_extent_thrw_tdrw_free(runtime,&cohort->metas);
  skc_runtime_host_temp_free(runtime,cohort,cohort->id);

  // release the raster builder
  skc_raster_builder_pfn_release(impl);

  //
  // ALLOCATED RESOURCES
  //
  // path_ids          -
  // raster_ids        -
  // transforms        -
  // clips             -
  // fill_cmds         -
  // cq                -
  // cohort atomics    -
  // cmds              -
  // keys              -
  // meta              -
  //
}

//
//
//

static
void
skc_raster_cohort_prefix_release(skc_grid_t const grid)
{
  // FIXME -- note that pfn_dispose can be accomplished here

  // release the grid
  skc_grid_complete(grid);
}

static
void
skc_raster_cohort_prefix_cb(cl_event event, cl_int status, skc_grid_t const grid)
{
  SKC_CL_CB(status);

  struct skc_raster_cohort * const cohort    = skc_grid_get_data(grid);
  struct skc_scheduler     * const scheduler = cohort->impl->runtime->scheduler;

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(scheduler,skc_raster_cohort_prefix_release,grid);
}

//
//
//

#if 0
static
int cmp64(const void * ptr_a, const void * ptr_b)
{
  skc_ulong const a = *(const skc_ulong *)ptr_a;
  skc_ulong const b = *(const skc_ulong *)ptr_b;

  if (a < b) return -1;
  if (a > b) return +1;
  else       return  0;
}
#endif

//
//
//

static
void
skc_raster_cohort_sort_prefix(skc_grid_t const grid)
{
  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        a
  // clips             a
  // fill_cmds         -
  // cq                a
  // cohort atomics    a
  // cmds              a
  // keys              a
  // meta              -
  //

  // use the backpointers
  struct skc_raster_cohort       * const cohort  = skc_grid_get_data(grid);
  struct skc_raster_builder_impl * const impl    = cohort->impl;
  struct skc_runtime             * const runtime = impl->runtime;

  // release transforms
  skc_extent_phw1g_tdrNs_snap_free(runtime,&cohort->transforms);

  // release clips
  skc_extent_phw1g_tdrNs_snap_free(runtime,&cohort->clips);

  // release expanded cmds
  skc_extent_tdrw_free(runtime,&cohort->cmds);

  // alloc the snapshost -- could be zero-sized
  skc_extent_phrwg_tdrNs_snap_alloc(runtime,
                                    &impl->raster_ids,
                                    &cohort->raster_ids,
                                    cohort->cq,NULL);

  // will never be zero
  skc_uint const rasters = skc_extent_ring_snap_count(cohort->raster_ids.snap);

  // acquire fixed-size device-side extent
  skc_extent_tdrw_alloc(runtime,
                        &cohort->metas,
                        sizeof(struct skc_raster_cohort_meta));

  // skc_extent_thrw_tdrw_alloc(runtime,
  //                            &cohort->metas,
  //                            sizeof(struct skc_raster_cohort_meta));

  // zero the metas
  skc_extent_tdrw_zero(&cohort->metas,cohort->cq,NULL);

  // get the read-only host copy of the device atomics
  struct skc_raster_cohort_atomic const * const atomics = cohort->atomics.hr;

  //
  // SORT
  //
  if (atomics->keys > 0)
    {
#ifndef NDEBUG
      fprintf(stderr,"raster cohort sort: %u\n",atomics->keys);
#endif

      //
      //
      //
      uint32_t keys_padded_in, keys_padded_out;

      hs_cl_pad(runtime->hs,atomics->keys,&keys_padded_in,&keys_padded_out);

      hs_cl_sort(runtime->hs,
                 cohort->cq,
                 0,NULL,NULL,
                 cohort->keys.drw,
                 NULL,
                 atomics->keys,
                 keys_padded_in,
                 keys_padded_out,
                 false);

      cl(SetKernelArg(impl->kernels.segment,0,SKC_CL_ARG(cohort->keys.drw)));
      cl(SetKernelArg(impl->kernels.segment,1,SKC_CL_ARG(cohort->metas.drw)));

#ifndef NDEBUG
      fprintf(stderr,"post-sort\n");
#endif

      // find start of each tile
      skc_device_enqueue_kernel(runtime->device,
                                SKC_DEVICE_KERNEL_ID_SEGMENT_TTRK,
                                cohort->cq,
                                impl->kernels.segment,
                                atomics->keys,
                                0,NULL,NULL);

#ifndef NDEBUG
      fprintf(stderr,"post-segment\n");
#endif

      //
      // DELETE ALL THIS WHEN READY
      //

#if 0
      //
      //
      //
      cl(Finish(cohort->cq));

      // map keys to host
      union skc_ttrk * const keys = skc_extent_thrw_tdrw_map(&cohort->keys,
                                                             cohort->cq,
                                                             NULL);
      // map meta to host
      struct skc_raster_cohort_meta * const metas = skc_extent_thrw_tdrw_map(&cohort->metas,
                                                                             cohort->cq,
                                                                             NULL);
      // block until done
      cl(Finish(cohort->cq));

      // sort keys
      qsort(keys,atomics->keys,sizeof(*keys),cmp64);

      // mask to determine if rk id is a new block
      skc_uint const subblock_mask = runtime->config->block.subblocks - 1;

      //
      // some counters
      //
      union skc_raster_cohort_meta_in meta_in = {
        .blocks = 0,
        .offset = 0,
        .pk     = 0,
        .rk     = 0
      };

      // get first key
      union skc_ttrk curr = keys[0];

      skc_uint ii=0, jj=0;

      // for all TTRK keys
      while (true)
        {
          // increment ttrk count
          meta_in.rk += 1;

          // was this a new block?
          if ((curr.u32v2.lo & subblock_mask) == 0)
            meta_in.blocks += 1;

          // break if we're out of keys
          if (++ii >= atomics->keys)
            break;

          // otherwise, process next key
          union skc_ttrk const next = keys[ii];

          // if new cohort then save curr meta and init next meta
          if (next.cohort != curr.cohort)
            {
              fprintf(stderr,"[ %u, %u, %u, %u ]\n",
                      meta_in.blocks,
                      meta_in.offset,
                      meta_in.pk,
                      meta_in.rk);

              // store back to buffer
              metas->inout[curr.cohort].in = meta_in;

              // update meta_in
              meta_in.blocks = 0;
              meta_in.offset = ii;
              meta_in.pk     = 0;
              meta_in.rk     = 0;
            }
          // otherwise, if same y but new x then increment TTPK count
          else if ((next.y == curr.y) && (next.x != curr.x))
            {
              meta_in.pk += 1;

#if 0
              fprintf(stderr,"%3u : %3u : ( %3u, %3u ) -> ( %3u )\n",
                      jj++,curr.cohort,curr.y,curr.x,next.x);
#endif
            }

#if 0
          fprintf(stderr,"( %3u, %3u )\n",next.y,next.x);
#endif

          curr = next;
        }

      fprintf(stderr,"[ %u, %u, %u, %u ]\n",
              meta_in.blocks,
              meta_in.offset,
              meta_in.pk,
              meta_in.rk);

      // store back to buffer
      metas->inout[curr.cohort].in = meta_in;


      // unmap
      skc_extent_thrw_tdrw_unmap(&cohort->keys,
                                 keys,
                                 cohort->cq,
                                 NULL);

      // unmap
      skc_extent_thrw_tdrw_unmap(&cohort->metas,
                                 metas,
                                 cohort->cq,
                                 NULL);
#endif
    }

#ifndef NDEBUG
  fprintf(stderr,"rasters_alloc: %u\n",rasters);
#endif

  //
  // RASTER ALLOC/INIT
  //
  cl(SetKernelArg(impl->kernels.rasters_alloc,0,SKC_CL_ARG(runtime->block_pool.atomics.drw)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,1,SKC_CL_ARG(runtime->block_pool.ids.drw)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,2,SKC_CL_ARG(runtime->block_pool.size->ring_mask)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,3,SKC_CL_ARG(runtime->handle_pool.map.drw)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,4,SKC_CL_ARG(cohort->metas.drw)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,5,SKC_CL_ARG(cohort->raster_ids.drN)));
  cl(SetKernelArg(impl->kernels.rasters_alloc,6,SKC_CL_ARG(rasters)));

  skc_device_enqueue_kernel(runtime->device,
                            SKC_DEVICE_KERNEL_ID_RASTERS_ALLOC,
                            cohort->cq,
                            impl->kernels.rasters_alloc,
                            rasters,
                            0,NULL,NULL);

#ifndef NDEBUG
  fprintf(stderr,"post-alloc\n");
#endif

  //
  // PREFIX
  //
  cl(SetKernelArg(impl->kernels.prefix,0,SKC_CL_ARG(runtime->block_pool.atomics.drw)));
  cl(SetKernelArg(impl->kernels.prefix,1,SKC_CL_ARG(runtime->block_pool.ids.drw)));
  cl(SetKernelArg(impl->kernels.prefix,2,SKC_CL_ARG(runtime->block_pool.blocks.drw)));
  cl(SetKernelArg(impl->kernels.prefix,3,SKC_CL_ARG(runtime->block_pool.size->ring_mask)));

  cl(SetKernelArg(impl->kernels.prefix,4,SKC_CL_ARG(cohort->keys.drw)));
  cl(SetKernelArg(impl->kernels.prefix,5,SKC_CL_ARG(runtime->handle_pool.map.drw)));

  cl(SetKernelArg(impl->kernels.prefix,6,SKC_CL_ARG(cohort->metas.drw)));
  cl(SetKernelArg(impl->kernels.prefix,7,SKC_CL_ARG(rasters)));

  cl_event complete;

  skc_device_enqueue_kernel(runtime->device,
                            SKC_DEVICE_KERNEL_ID_PREFIX,
                            cohort->cq,
                            impl->kernels.prefix,
                            rasters,
                            0,NULL,
                            &complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_raster_cohort_prefix_cb,grid));
  cl(ReleaseEvent(complete));

#ifndef NDEBUG
  fprintf(stderr,"post-prefix\n");
#endif

  // flush command queue
  cl(Flush(cohort->cq));

  //
  // ALLOCATED RESOURCES
  //
  // path_ids          a
  // raster_ids        a
  // transforms        -
  // clips             -
  // fill_cmds         -
  // cq                a
  // cohort atomics    a
  // cmds              -
  // keys              a
  // meta              a
  //
}

static
void
skc_raster_cohort_rasterize_cb(cl_event event, cl_int status, skc_grid_t const grid)
{
  SKC_CL_CB(status);

  struct skc_raster_cohort * const cohort = skc_grid_get_data(grid);

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(cohort->impl->runtime->scheduler,skc_raster_cohort_sort_prefix,grid);
}

static
void
skc_raster_cohort_rasterize(skc_grid_t const grid)
{
  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        i
  // clips             i
  // fill_cmds         s
  // cq                a
  // cohort atomics    a
  // cmds              a
  // cmds_quad         a
  // cmds_cubic        a
  // keys              -
  // meta              -

  // use the backpointers
  struct skc_raster_cohort       * const cohort  = skc_grid_get_data(grid);
  struct skc_raster_builder_impl * const impl    = cohort->impl;
  struct skc_runtime             * const runtime = impl->runtime;

  //
  // RELEASED RESOURCES
  //
  // cmds       snap
  //

  // release the cmds extent and snap since it's only used by the expand stage
  skc_extent_phw1g_tdrNs_snap_free(runtime,&cohort->fill_cmds);

  //
  // NEW ALLOCATED RESOURCES
  //
  // transforms snap
  // clips snap
  // ttrk keys
  //
  skc_extent_phw1g_tdrNs_snap_alloc(runtime,
                                    &impl->transforms,
                                    &cohort->transforms,
                                    cohort->cq,NULL);

  skc_extent_phw1g_tdrNs_snap_alloc(runtime,
                                    &impl->clips,
                                    &cohort->clips,
                                    cohort->cq,NULL);

  // acquire device-side extent
  skc_extent_tdrw_alloc(runtime,
                        &cohort->keys,
                        sizeof(union skc_ttrk) * runtime->config->raster_cohort.rasterize.keys);

  // skc_extent_thrw_tdrw_alloc(runtime,
  //                            &cohort->keys,
  //                            sizeof(union skc_ttrk) * runtime->config->raster_cohort.rasterize.keys);

  //
  // acquire out-of-order command queue
  //
  // and launch up to 3 kernels
  //
  // for each kernel:
  //
  //   set runtime "global" kernel args:
  //
  //   - block pool atomics
  //   - block pool extent
  //
  //   set cohort "local" kernel args:
  //
  //   - atomics
  //   - cmds
  //
  // enqueue barrier
  // enqueue copy back of atomics on the command queue
  // set callback on copy back event
  // release command queue
  //
  struct skc_raster_cohort_atomic const * const atomics = cohort->atomics.hr;

  if (atomics->cmds > 0)
    {
      cl(SetKernelArg(impl->kernels.rasterize_all,0,SKC_CL_ARG(runtime->block_pool.atomics.drw)));
      cl(SetKernelArg(impl->kernels.rasterize_all,1,SKC_CL_ARG(runtime->block_pool.blocks.drw)));
      cl(SetKernelArg(impl->kernels.rasterize_all,2,SKC_CL_ARG(runtime->block_pool.ids.drw)));
      cl(SetKernelArg(impl->kernels.rasterize_all,3,SKC_CL_ARG(runtime->block_pool.size->ring_mask)));

      cl(SetKernelArg(impl->kernels.rasterize_all,4,SKC_CL_ARG(cohort->atomics.drw)));
      cl(SetKernelArg(impl->kernels.rasterize_all,5,SKC_CL_ARG(cohort->keys.drw)));

      cl(SetKernelArg(impl->kernels.rasterize_all,6,SKC_CL_ARG(cohort->transforms.drN)));
      cl(SetKernelArg(impl->kernels.rasterize_all,7,SKC_CL_ARG(cohort->clips.drN)));
      cl(SetKernelArg(impl->kernels.rasterize_all,8,SKC_CL_ARG(cohort->cmds.drw)));
      cl(SetKernelArg(impl->kernels.rasterize_all,9,SKC_CL_ARG(atomics->cmds)));

      skc_device_enqueue_kernel(runtime->device,
                                SKC_DEVICE_KERNEL_ID_RASTERIZE_ALL,
                                cohort->cq,
                                impl->kernels.rasterize_all,
                                atomics->cmds,
                                0,NULL,NULL);
    }

  //
  // copyback number of TTSK keys
  //
  cl_event complete;

  skc_extent_thr_tdrw_read(&cohort->atomics,cohort->cq,&complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_raster_cohort_rasterize_cb,grid));
  cl(ReleaseEvent(complete));

  // flush command queue
  cl(Flush(cohort->cq));

  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        a
  // clips             a
  // fill_cmds         -
  // cq                a
  // cohort atomics    a
  // cmds              a
  // keys              a
  // meta              -
}

static
void
skc_raster_cohort_fills_expand_cb(cl_event event, cl_int status, skc_grid_t const grid)
{
  SKC_CL_CB(status);

  struct skc_raster_cohort * const cohort = skc_grid_get_data(grid);

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(cohort->impl->runtime->scheduler,skc_raster_cohort_rasterize,grid);
}

static
void
skc_raster_builder_cohort_grid_pfn_execute(skc_grid_t const grid)
{
  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        i
  // clips             i
  // fill_cmds         i
  // cq                -
  // cohort atomics    -
  // cmds              -
  // keys              -
  // meta              -
  //

  // allocate the cohort
  struct skc_raster_cohort       * const cohort  = skc_grid_get_data(grid);

  // get impl
  struct skc_raster_builder_impl * const impl    = cohort->impl;
  struct skc_runtime             * const runtime = impl->runtime;

  // acquire in-order cq
  cohort->cq = skc_runtime_acquire_cq_in_order(runtime);

  // alloc the snapshot -- could be zero-sized
  skc_extent_phw1g_tdrNs_snap_alloc(runtime,
                                    &impl->fill_cmds,
                                    &cohort->fill_cmds,
                                    cohort->cq,NULL);

  // flush the cq to get the fill running
  // cl(Flush(cohort->cq));

  // create split atomics
  skc_extent_thr_tdrw_alloc(runtime,&cohort->atomics,sizeof(struct skc_raster_cohort_atomic));

  // zero the atomics
  skc_extent_thr_tdrw_zero(&cohort->atomics,cohort->cq,NULL);

  // get config
  struct skc_config const * const config = runtime->config;

  // acquire device-side extents
  skc_extent_tdrw_alloc(runtime,
                        &cohort->cmds,
                        sizeof(union skc_cmd_rasterize) * config->raster_cohort.expand.cmds);

  //
  // FILLS EXPAND
  //
  // need result of cmd counts before launching RASTERIZE grids
  //
  // - OpenCL 1.2: copy atomic counters back to host and launch RASTERIZE grids from host
  // - OpenCL 2.x: have a kernel size and launch RASTERIZE grids from device
  // - or launch a device-wide grid that feeds itself but that's unsatisfying
  //

  // how many commands?  could be zero
  skc_uint const work_size = skc_extent_ring_snap_count(cohort->fill_cmds.snap);

  if (work_size > 0)
    {
      cl(SetKernelArg(impl->kernels.fills_expand,0,SKC_CL_ARG(impl->runtime->block_pool.blocks.drw)));
      cl(SetKernelArg(impl->kernels.fills_expand,1,SKC_CL_ARG(cohort->atomics.drw)));
      cl(SetKernelArg(impl->kernels.fills_expand,2,SKC_CL_ARG(runtime->handle_pool.map.drw)));
      cl(SetKernelArg(impl->kernels.fills_expand,3,SKC_CL_ARG(cohort->fill_cmds.drN)));
      cl(SetKernelArg(impl->kernels.fills_expand,4,SKC_CL_ARG(cohort->cmds.drw)));

      skc_device_enqueue_kernel(runtime->device,
                                SKC_DEVICE_KERNEL_ID_FILLS_EXPAND,
                                cohort->cq,
                                impl->kernels.fills_expand,
                                work_size,
                                0,NULL,NULL);
    }

  //
  // copyback number of rasterization commands
  //
  cl_event complete;

  skc_extent_thr_tdrw_read(&cohort->atomics,cohort->cq,&complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_raster_cohort_fills_expand_cb,grid));
  cl(ReleaseEvent(complete));

  // flush command queue
  cl(Flush(cohort->cq));

  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        i
  // clips             i
  // fill_cmds         s
  // cq                a
  // cohort atomics    a
  // cmds              a
  // keys              -
  // meta              -
  //
}

//
// move grid into waiting state
//
// this entails allocating a cohort from the temporary extent
//

static
void
skc_raster_builder_cohort_grid_pfn_waiting(skc_grid_t const grid)
{
  // get the impl
  struct skc_raster_builder_impl * const impl    = skc_grid_get_data(grid);
  struct skc_runtime             * const runtime = impl->runtime;

  // retain the raster builder
  impl->raster_builder->refcount += 1;

  // allocate the ephemeral/temp cohort
  skc_subbuf_id_t id;

  struct skc_raster_cohort * const cohort =
    skc_runtime_host_temp_alloc(runtime,
                                SKC_MEM_FLAGS_READ_WRITE,
                                sizeof(*cohort),
                                &id,
                                NULL);

  // save the id and backpointer
  cohort->id   = id;
  cohort->impl = impl;

  // set grid data -- replaces impl
  skc_grid_set_data(grid,cohort);

  //
  // ACQUIRE RESOURCES FOR THE COHORT
  //

  struct skc_raster_builder * const raster_builder = impl->raster_builder;

  // immediately take snapshots of all rings -- these are very inexpensive operations
  skc_extent_phrwg_thr1s_snap_init(runtime,&raster_builder->path_ids  .ring,&cohort->path_ids);
  skc_extent_phw1g_tdrNs_snap_init(runtime,&raster_builder->transforms.ring,&cohort->transforms);
  skc_extent_phw1g_tdrNs_snap_init(runtime,&raster_builder->clips     .ring,&cohort->clips);
  skc_extent_phw1g_tdrNs_snap_init(runtime,&raster_builder->fill_cmds .ring,&cohort->fill_cmds);
  skc_extent_phrwg_tdrNs_snap_init(runtime,&raster_builder->raster_ids.ring,&cohort->raster_ids);

  //
  // ALLOCATED RESOURCES
  //
  // path_ids          i
  // raster_ids        i
  // transforms        i
  // clips             i
  // fill_cmds         i
  // cq                -
  // cohort atomics    -
  // cmds              -
  // keys              -
  // meta              -
  //
}

//
//
//

static
void
skc_raster_builder_cohort_create(struct skc_raster_builder_impl * const impl)
{
  // attach a grid
  impl->cohort = SKC_GRID_DEPS_ATTACH(impl->runtime->deps,
                                      &impl->cohort,
                                      impl,
                                      skc_raster_builder_cohort_grid_pfn_waiting,
                                      skc_raster_builder_cohort_grid_pfn_execute,
                                      skc_raster_builder_cohort_grid_pfn_dispose);
}

//
//
//

static
skc_err
skc_raster_builder_pfn_add(struct skc_raster_builder_impl * const impl,
                           skc_path_t               const *       paths,
                           skc_uint                               count)
{
  // validate and retain the path
  skc_err err;

  err = skc_runtime_handle_device_validate_retain(impl->runtime,
                                                  SKC_TYPED_HANDLE_TYPE_IS_PATH,
                                                  paths,
                                                  count);

  if (err)
    return err;

  skc_runtime_handle_device_retain(impl->runtime,paths,count);

  // make sure there is a grid
  if (impl->cohort == NULL) {
    skc_raster_builder_cohort_create(impl);
  }

  // declare rasterization grid happens after path
  while (count-- > 0)
    skc_grid_happens_after_handle(impl->cohort,SKC_TYPED_HANDLE_TO_HANDLE(*paths++));

  return SKC_ERR_SUCCESS;
}

//
//
//

static
void
skc_raster_builder_pfn_end(struct skc_raster_builder_impl * const impl, skc_raster_t * const raster)
{
  //
  // acquire host-managed path raster handle and bump reference count
  // to 2 handles will be released (reduced to 1) once the rasters are
  // completely rasterized
  //
  *raster = skc_runtime_handle_device_acquire(impl->runtime);

  // make sure there is a grid
  if (impl->cohort == NULL) {
    skc_raster_builder_cohort_create(impl);
  }

  // map a handle to a grid
  skc_grid_map(impl->cohort,*raster);
}

//
// snapshot the ring and lazily start the grid
//
// FIXME -- might want to revisit this and settle on an even more
// opaque implementation.  Some options:
//
//  - never let the SKC API expose a forced grid start
//  - make snapshots kick off a forced grid start
//  - be lazy all the time everywhere
//

static
void
skc_raster_builder_pfn_start(struct skc_raster_builder_impl * const impl)
{
  skc_grid_t const cohort = impl->cohort;

  if (cohort != NULL) {
    skc_grid_start(cohort);
  }
}

//
// NOTE: THIS MIGHT BE REMOVED
//

static
void
skc_raster_builder_pfn_force(struct skc_raster_builder_impl * const impl)
{
  skc_grid_t const cohort = impl->cohort;

  if (cohort != NULL) {
    skc_grid_force(cohort);
  }
}

//
//
//

skc_err
skc_raster_builder_cl_12_create(struct skc_context          * const context,
                                struct skc_raster_builder * * const raster_builder)
{
  struct skc_runtime * const runtime = context->runtime;

  // allocate raster builder
  (*raster_builder) = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(**raster_builder));

  // refcount
  (*raster_builder)->refcount = 1;

  // state
  SKC_ASSERT_STATE_INIT((*raster_builder),SKC_RASTER_BUILDER_STATE_READY);

  // allocate runtime raster builder
  struct skc_raster_builder_impl * const impl = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*impl));

  // save the impl
  (*raster_builder)->impl = impl;

  // intialize impl
  impl->raster_builder = (*raster_builder);
  impl->runtime        = runtime;
  impl->cohort         = NULL;

  // get config
  struct skc_config const * const config = runtime->config;

  skc_extent_phrwg_thr1s_alloc(runtime,&impl->path_ids  ,sizeof(skc_path_t         ) * config->raster_cohort.path_ids  .elem_count);
  skc_extent_phw1g_tdrNs_alloc(runtime,&impl->transforms,sizeof(union skc_transform) * config->raster_cohort.transforms.elem_count);
  skc_extent_phw1g_tdrNs_alloc(runtime,&impl->clips     ,sizeof(union skc_path_clip) * config->raster_cohort.clips     .elem_count);
  skc_extent_phw1g_tdrNs_alloc(runtime,&impl->fill_cmds ,sizeof(union skc_cmd_fill ) * config->raster_cohort.fill      .elem_count);
  skc_extent_phrwg_tdrNs_alloc(runtime,&impl->raster_ids,sizeof(skc_raster_t       ) * config->raster_cohort.raster_ids.elem_count);

  // retain the context
  //skc_context_retain(context);

  (*raster_builder)->context = context;

  (*raster_builder)->add     = skc_raster_builder_pfn_add;
  (*raster_builder)->end     = skc_raster_builder_pfn_end;
  (*raster_builder)->start   = skc_raster_builder_pfn_start;
  (*raster_builder)->force   = skc_raster_builder_pfn_force;
  (*raster_builder)->release = skc_raster_builder_pfn_release;

  // initialize raster builder with host-writable buffers
  (*raster_builder)->path_ids  .extent = impl->path_ids.hrw;
  (*raster_builder)->transforms.extent = impl->transforms.hw1;
  (*raster_builder)->clips     .extent = impl->clips.hw1;
  (*raster_builder)->fill_cmds .extent = impl->fill_cmds.hw1;
  (*raster_builder)->raster_ids.extent = impl->raster_ids.hrw;

  //
  // the rings perform bookkeeping on the extents
  //
  // the ring snapshotting and checkpointing are necessary because
  // another part of the API can _force_ the raster cohort to flush
  // its work-in-progress commands but only up to a checkpointed
  // boundary
  //
  skc_extent_ring_init(&(*raster_builder)->path_ids.ring,
                       config->raster_cohort.path_ids.elem_count,
                       config->raster_cohort.path_ids.snap_count,
                       sizeof(skc_path_t));

  skc_extent_ring_init(&(*raster_builder)->transforms.ring,
                       config->raster_cohort.transforms.elem_count,
                       config->raster_cohort.transforms.snap_count,
                       sizeof(union skc_transform));

  skc_extent_ring_init(&(*raster_builder)->clips.ring,
                       config->raster_cohort.clips.elem_count,
                       config->raster_cohort.clips.snap_count,
                       sizeof(union skc_path_clip));

  skc_extent_ring_init(&(*raster_builder)->fill_cmds.ring,
                       config->raster_cohort.fill.elem_count,
                       config->raster_cohort.fill.snap_count,
                       sizeof(union skc_cmd_fill));

  skc_extent_ring_init(&(*raster_builder)->raster_ids.ring,
                       config->raster_cohort.raster_ids.elem_count,
                       config->raster_cohort.raster_ids.snap_count,
                       sizeof(skc_raster_t));

  //
  // acquire kernels
  //
  impl->kernels.fills_expand     = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_FILLS_EXPAND);
  impl->kernels.rasterize_all    = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_RASTERIZE_ALL);

#if 0
  impl->kernels.rasterize_lines  = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_RASTERIZE_LINES);
  impl->kernels.rasterize_quads  = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_RASTERIZE_QUADS);
  impl->kernels.rasterize_cubics = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_RASTERIZE_CUBICS);
#endif

  impl->kernels.segment          = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_SEGMENT_TTRK);
  impl->kernels.rasters_alloc    = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_RASTERS_ALLOC);
  impl->kernels.prefix           = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_PREFIX);

  return SKC_ERR_SUCCESS;
}

//
//
//
