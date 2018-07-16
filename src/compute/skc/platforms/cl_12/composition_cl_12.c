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

#include <stdlib.h>
#include <stdio.h>

#include "hs/cl/hs_cl_launcher.h"

#include "common/cl/assert_cl.h"

#include "composition_cl_12.h"
#include "config_cl.h"

#include "context.h"
#include "raster.h"
#include "handle.h"

#include "runtime_cl_12.h"

#include "common.h"
#include "tile.h"

//
// TTCK (32-BIT COMPARE) v1:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   18  |  7  |  7  |
//
//
// TTCK (32-BIT COMPARE) v2:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   15  |  9  |  8  |
//
//
// TTCK (64-BIT COMPARE) -- achieves 4K x 4K with an 8x16 tile:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          27          |    1   |    1   |   18  |  9  |  8  |
//

union skc_ttck
{
  skc_ulong   u64;
  skc_uint2   u32v2;

  struct {
    skc_uint  id         : SKC_TTCK_LO_BITS_ID;
    skc_uint  prefix     : SKC_TTCK_LO_BITS_PREFIX;
    skc_uint  escape     : SKC_TTCK_LO_BITS_ESCAPE;
    skc_uint  layer_lo   : SKC_TTCK_LO_BITS_LAYER;
    skc_uint  layer_hi   : SKC_TTCK_HI_BITS_LAYER;
    skc_uint  x          : SKC_TTCK_HI_BITS_X;
    skc_uint  y          : SKC_TTCK_HI_BITS_Y;
  };

  struct {
    skc_ulong na0        : SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE;
    skc_ulong layer      : SKC_TTCK_BITS_LAYER;
    skc_ulong na1        : SKC_TTCK_HI_BITS_YX;
  };

  struct {
    skc_uint  na2;
    skc_uint  na3        : SKC_TTCK_HI_BITS_LAYER;
    skc_uint  yx         : SKC_TTCK_HI_BITS_YX;
  };
};

//
// FIXME -- accept floats on host but convert to subpixel offsets
// before appending to command ring
//

#define SKC_PLACE_CMD_TX_CONVERT(f)  0
#define SKC_PLACE_CMD_TY_CONVERT(f)  0

//
// COMPOSITION PLACE
//
// This is a snapshot of the host-side command queue.
//
// Note that the composition command extent could be implemented as
// either a mapped buffer or simply copied to an ephemeral extent.
//
// This implementation may vary between compute platforms.
//

struct skc_composition_place
{
  struct skc_composition_impl      * impl;

  cl_command_queue                   cq;

  struct skc_extent_phw1g_tdrNs_snap cmds;

  skc_subbuf_id_t                    id;
};

//
// Forward declarations
//

static
void
skc_composition_unseal_block(struct skc_composition_impl * const impl,
                             skc_bool                      const block);

//
//
//

static
void
skc_composition_pfn_release(struct skc_composition_impl * const impl)
{
  if (--impl->composition->ref_count != 0)
    return;

  //
  // otherwise, dispose of all resources
  //

  // the unsealed state is a safe state to dispose of resources
  skc_composition_unseal_block(impl,true); // block

  struct skc_runtime * const runtime = impl->runtime;

  // free host composition
  skc_runtime_host_perm_free(runtime,impl->composition);

  // release the cq
  skc_runtime_release_cq_in_order(runtime,impl->cq);

  // release kernels
  cl(ReleaseKernel(impl->kernels.place));
  cl(ReleaseKernel(impl->kernels.segment));

  // release extents
  skc_extent_phw1g_tdrNs_free(runtime,&impl->cmds.extent);
  skc_extent_phrw_free       (runtime,&impl->saved.extent);
  skc_extent_phr_pdrw_free   (runtime,&impl->atomics);

  skc_extent_pdrw_free       (runtime,&impl->keys);
  skc_extent_pdrw_free       (runtime,&impl->offsets);

  // free composition impl
  skc_runtime_host_perm_free(runtime,impl);
}

//
//
//

static
void
skc_composition_place_grid_pfn_dispose(skc_grid_t const grid)
{
  struct skc_composition_place * const place   = skc_grid_get_data(grid);
  struct skc_composition_impl  * const impl    = place->impl;
  struct skc_runtime           * const runtime = impl->runtime;

  // release cq
  skc_runtime_release_cq_in_order(runtime,place->cq);

  // unmap the snapshot (could be a copy)
  skc_extent_phw1g_tdrNs_snap_free(runtime,&place->cmds);

  // release place struct
  skc_runtime_host_temp_free(runtime,place,place->id);

  // release impl
  skc_composition_pfn_release(impl);
}

//
//
//

static
void
skc_composition_place_read_complete(skc_grid_t const grid)
{
  skc_grid_complete(grid);
}

static
void
skc_composition_place_read_cb(cl_event event, cl_int status, skc_grid_t const grid)
{
  SKC_CL_CB(status);

  struct skc_composition_place * const place     = skc_grid_get_data(grid);
  struct skc_composition_impl  * const impl      = place->impl;
  struct skc_runtime           * const runtime   = impl->runtime;
  struct skc_scheduler         * const scheduler = runtime->scheduler;

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(scheduler,skc_composition_place_read_complete,grid);
}

static
void
skc_composition_place_grid_pfn_execute(skc_grid_t const grid)
{
  //
  // FILLS EXPAND
  //
  // need result of cmd counts before launching RASTERIZE grids
  //
  // - OpenCL 1.2: copy atomic counters back to host and launch RASTERIZE grids from host
  // - OpenCL 2.x: have a kernel size and launch RASTERIZE grids from device
  // - or launch a device-wide grid that feeds itself but that's unsatisfying
  //
  struct skc_composition_place * const place   = skc_grid_get_data(grid);
  struct skc_composition_impl  * const impl    = place->impl;
  struct skc_runtime           * const runtime = impl->runtime;

  skc_uint  const work_size = skc_extent_ring_snap_count(place->cmds.snap);
  skc_uint4 const clip      = { 0, 0, SKC_UINT_MAX, SKC_UINT_MAX };

  // initialize kernel args
  cl(SetKernelArg(impl->kernels.place,0,SKC_CL_ARG(impl->runtime->block_pool.blocks.drw)));
  cl(SetKernelArg(impl->kernels.place,1,SKC_CL_ARG(impl->atomics.drw)));
  cl(SetKernelArg(impl->kernels.place,2,SKC_CL_ARG(impl->keys.drw)));
  cl(SetKernelArg(impl->kernels.place,3,SKC_CL_ARG(place->cmds.drN)));
  cl(SetKernelArg(impl->kernels.place,4,SKC_CL_ARG(runtime->handle_pool.map.drw)));
  cl(SetKernelArg(impl->kernels.place,5,SKC_CL_ARG(clip))); // FIXME -- convert the clip to yx0/yx1 format
  cl(SetKernelArg(impl->kernels.place,6,SKC_CL_ARG(work_size)));

  // launch kernel
  skc_device_enqueue_kernel(runtime->device,
                            SKC_DEVICE_KERNEL_ID_PLACE,
                            place->cq,
                            impl->kernels.place,
                            work_size,
                            0,NULL,NULL);
  //
  // copy atomics back after every place launch
  //
  cl_event complete;

  skc_extent_phr_pdrw_read(&impl->atomics,place->cq,&complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_composition_place_read_cb,grid));
  cl(ReleaseEvent(complete));

  // flush command queue
  cl(Flush(place->cq));
}

//
//
//

static
void
skc_composition_snap(struct skc_composition_impl * const impl)
{
  skc_composition_retain(impl->composition);

  skc_subbuf_id_t id;

  struct skc_composition_place * const place = skc_runtime_host_temp_alloc(impl->runtime,
                                                                           SKC_MEM_FLAGS_READ_WRITE,
                                                                           sizeof(*place),&id,NULL);

  // save the subbuf id
  place->id = id;

  // save backpointer
  place->impl = impl;

  // set grid data
  skc_grid_set_data(impl->grids.place,place);

  // acquire command queue
  place->cq = skc_runtime_acquire_cq_in_order(impl->runtime);

  // checkpoint the ring
  skc_extent_ring_checkpoint(&impl->cmds.ring);

  // make a snapshot
  skc_extent_phw1g_tdrNs_snap_init(impl->runtime,&impl->cmds.ring,&place->cmds);

  // unmap the snapshot (could be a copy)
  skc_extent_phw1g_tdrNs_snap_alloc(impl->runtime,
                                    &impl->cmds.extent,
                                    &place->cmds,
                                    place->cq,
                                    NULL);

  skc_grid_force(impl->grids.place);
}

//
//
//

static
void
skc_composition_pfn_seal(struct skc_composition_impl * const impl)
{
  // return if sealing or sealed
  if (impl->state >= SKC_COMPOSITION_STATE_SEALING)
    return;

  struct skc_runtime   * const runtime   = impl->runtime;
  struct skc_scheduler * const scheduler = runtime->scheduler;

  //
  // otherwise, wait for UNSEALING > UNSEALED transition
  //
  if (impl->state == SKC_COMPOSITION_STATE_UNSEALING)
    {
      SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_COMPOSITION_STATE_UNSEALED);
    }
  else // or we were already unsealed
    {
      // flush is there is work in progress
      skc_uint const count = skc_extent_ring_wip_count(&impl->cmds.ring);

      if (count > 0) {
        skc_composition_snap(impl);
      }
    }

  //
  // now unsealed so we need to start sealing...
  //
  impl->state = SKC_COMPOSITION_STATE_SEALING;

  //
  // the seal operation implies we should force start all dependencies
  // that are still in a ready state
  //
  skc_grid_force(impl->grids.sort);
}

//
//
//

void
skc_composition_sort_execute_complete(struct skc_composition_impl * const impl)
{
  // we're sealed
  impl->state = SKC_COMPOSITION_STATE_SEALED;

  // this grid is done
  skc_grid_complete(impl->grids.sort);
}

static
void
skc_composition_sort_execute_cb(cl_event event, cl_int status, struct skc_composition_impl * const impl)
{
  SKC_CL_CB(status);

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(impl->runtime->scheduler,skc_composition_sort_execute_complete,impl);
}

static
void
skc_composition_sort_grid_pfn_execute(skc_grid_t const grid)
{
  struct skc_composition_impl * const impl    = skc_grid_get_data(grid);
  struct skc_runtime          * const runtime = impl->runtime;

  // we should be sealing
  assert(impl->state == SKC_COMPOSITION_STATE_SEALING);

  struct skc_place_atomics * const atomics = impl->atomics.hr;

#ifndef NDEBUG
  fprintf(stderr,"composition sort: %u\n",atomics->keys);
#endif

  if (atomics->keys > 0)
    {
      uint32_t keys_padded_in, keys_padded_out;

      hs_cl_pad(runtime->hs,atomics->keys,&keys_padded_in,&keys_padded_out);

      hs_cl_sort(impl->runtime->hs,
                 impl->cq,
                 0,NULL,NULL,
                 impl->keys.drw,
                 NULL,
                 atomics->keys,
                 keys_padded_in,
                 keys_padded_out,
                 false);

      cl(SetKernelArg(impl->kernels.segment,0,SKC_CL_ARG(impl->keys.drw)));
      cl(SetKernelArg(impl->kernels.segment,1,SKC_CL_ARG(impl->offsets.drw)));
      cl(SetKernelArg(impl->kernels.segment,2,SKC_CL_ARG(impl->atomics.drw)));

      // find start of each tile
      skc_device_enqueue_kernel(runtime->device,
                                SKC_DEVICE_KERNEL_ID_SEGMENT_TTCK,
                                impl->cq,
                                impl->kernels.segment,
                                atomics->keys,
                                0,NULL,NULL);
    }

  cl_event complete;

  // next stage needs to know number of key segments
  skc_extent_phr_pdrw_read(&impl->atomics,impl->cq,&complete);

  // register a callback
  cl(SetEventCallback(complete,CL_COMPLETE,skc_composition_sort_execute_cb,impl));
  cl(ReleaseEvent(complete));

  // flush cq
  cl(Flush(impl->cq));
}

//
//
//

static
void
skc_composition_raster_release(struct skc_composition_impl * const impl)
{
  //
  // reference counts to rasters can only be released when the
  // composition is unsealed and the atomics are reset.
  //
  skc_runtime_raster_device_release(impl->runtime,
                                    impl->saved.extent.hrw,
                                    impl->saved.count);
  // reset count
  impl->saved.count = 0;
}

//
//
//

static
void
skc_composition_unseal_block(struct skc_composition_impl * const impl,
                             skc_bool                      const block)
{
  // return if already unsealed
  if (impl->state == SKC_COMPOSITION_STATE_UNSEALED)
    return;

  //
  // otherwise, we're going to need to pump the scheduler
  //
  struct skc_scheduler * const scheduler = impl->runtime->scheduler;

  //
  // wait for UNSEALING > UNSEALED transition
  //
  if (impl->state == SKC_COMPOSITION_STATE_UNSEALING)
    {
      if (block) {
        SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_COMPOSITION_STATE_UNSEALED);
      }
      return;
    }

  //
  // wait for SEALING > SEALED transition ...
  //
  if (impl->state == SKC_COMPOSITION_STATE_SEALING)
    {
      // wait if sealing
      SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_COMPOSITION_STATE_SEALED);
    }

  // wait for rendering locks to be released
  SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->lock_count > 0);

  //
  // no need to visit UNSEALING state with this implementation
  //

  // acquire a new grid
  impl->grids.sort = SKC_GRID_DEPS_ATTACH(impl->runtime->deps,
                                          NULL,  // the composition state guards this
                                          impl,
                                          NULL,  // no waiting
                                          skc_composition_sort_grid_pfn_execute,
                                          NULL); // no dispose

  // mark composition as unsealed
  impl->state = SKC_COMPOSITION_STATE_UNSEALED;
}

//
// can only be called on a composition that was just unsealed
//
static
void
skc_composition_reset(struct skc_composition_impl * const impl)
{
  // zero the atomics
  skc_extent_phr_pdrw_zero(&impl->atomics,impl->cq,NULL);

  // flush it
  cl(Flush(impl->cq));

  // release all the rasters
  skc_composition_raster_release(impl);
}

static
void
skc_composition_unseal_block_reset(struct skc_composition_impl * const impl,
                                   skc_bool                      const block,
                                   skc_bool                      const reset)
{
  skc_composition_unseal_block(impl,block);

  if (reset) {
    skc_composition_reset(impl);
  }
}

//
//
//

static
void
skc_composition_pfn_unseal(struct skc_composition_impl * const impl, skc_bool const reset)
{
  skc_composition_unseal_block_reset(impl,false,reset);
}

//
// only needs to create a grid
//

static
void
skc_composition_place_create(struct skc_composition_impl * const impl)
{
  // acquire a grid
  impl->grids.place = SKC_GRID_DEPS_ATTACH(impl->runtime->deps,
                                           &impl->grids.place,
                                           NULL,
                                           NULL, // no waiting
                                           skc_composition_place_grid_pfn_execute,
                                           skc_composition_place_grid_pfn_dispose);

  // assign happens-after relationship
  skc_grid_happens_after_grid(impl->grids.sort,impl->grids.place);
}


static
skc_err
skc_composition_pfn_place(struct skc_composition_impl * const impl,
                          skc_raster_t          const *       rasters,
                          skc_layer_id          const *       layer_ids,
                          skc_float             const *       txs,
                          skc_float             const *       tys,
                          skc_uint                            count)
{
  // block and yield if not unsealed
  skc_composition_unseal_block(impl,true);

  //
  // validate and retain all rasters
  //
  skc_err err;

  err = skc_runtime_handle_device_validate_retain(impl->runtime,
                                                  SKC_TYPED_HANDLE_TYPE_IS_RASTER,
                                                  rasters,
                                                  count);
  if (err)
    return err;

  skc_runtime_handle_device_retain(impl->runtime,rasters,count);

  //
  // save the stripped handles
  //
  skc_raster_t * saved = impl->saved.extent.hrw;

  saved             += impl->saved.count;
  impl->saved.count += count;

  for (skc_uint ii=0; ii<count; ii++) {
    saved[ii] = SKC_TYPED_HANDLE_TO_HANDLE(*rasters++);
  }

  //
  // - declare the place grid happens after the raster
  // - copy place commands into ring
  //
  do {
    skc_uint rem;

    // find out how much room is left in then ring's snap
    // if the place ring is full -- let it drain
    SKC_SCHEDULER_WAIT_WHILE(impl->runtime->scheduler,(rem = skc_extent_ring_wip_rem(&impl->cmds.ring)) == 0);

    // append commands
    skc_uint avail = min(rem,count);

    // decrement count
    count -= avail;

    // launch a place kernel after copying commands?
    skc_bool const is_wip_full = (avail == rem);

    // if there is no place grid then create one
    if (impl->grids.place == NULL)
      {
        skc_composition_place_create(impl);
      }

    //
    // FIXME -- OPTIMIZATION? -- the ring_wip_index_inc() test can
    // be avoided by splitting into at most two intervals. It should
    // be plenty fast as is though so leave for now.
    //
    union skc_cmd_place * const cmds = impl->cmds.extent.hw1;

    if ((txs == NULL) && (tys == NULL))
      {
        while (avail-- > 0)
          {
            skc_raster_t const raster = *saved++;

            skc_grid_happens_after_handle(impl->grids.place,raster);

            cmds[skc_extent_ring_wip_index_inc(&impl->cmds.ring)] =
              (union skc_cmd_place){ raster, *layer_ids++, 0, 0 };
          }
      }
    else if (txs == NULL)
      {
        while (avail-- > 0)
          {
            skc_raster_t const raster = *saved++;

            skc_grid_happens_after_handle(impl->grids.place,raster);

            cmds[skc_extent_ring_wip_index_inc(&impl->cmds.ring)] =
              (union skc_cmd_place){ raster,
                                     *layer_ids++,
                                     0,
                                     SKC_PLACE_CMD_TY_CONVERT(*tys++) };
          }
      }
    else if (tys == NULL)
      {
        while (avail-- > 0)
          {
            skc_raster_t const raster = *saved++;

            skc_grid_happens_after_handle(impl->grids.place,raster);

            cmds[skc_extent_ring_wip_index_inc(&impl->cmds.ring)] =
              (union skc_cmd_place){ raster,
                                     *layer_ids++,
                                     SKC_PLACE_CMD_TX_CONVERT(*txs++),
                                     0 };
          }
      }
    else
      {
        while (avail-- > 0)
          {
            skc_raster_t const raster = *saved++;

            skc_grid_happens_after_handle(impl->grids.place,raster);

            cmds[skc_extent_ring_wip_index_inc(&impl->cmds.ring)] =
              (union skc_cmd_place){ raster,
                                     *layer_ids++,
                                     SKC_PLACE_CMD_TX_CONVERT(*txs++),
                                     SKC_PLACE_CMD_TY_CONVERT(*tys++) };
          }
      }

    // launch place kernel?
    if (is_wip_full) {
      skc_composition_snap(impl);
    }
  } while (count > 0);

  return SKC_ERR_SUCCESS;
}

//
//
//

static
void
skc_composition_pfn_bounds(struct skc_composition_impl * const impl, skc_int bounds[4])
{
  //
  // FIXME -- not implemented yet
  //
  // impl bounds will be copied back after sealing
  //
  bounds[0] = SKC_INT_MIN;
  bounds[1] = SKC_INT_MIN;
  bounds[2] = SKC_INT_MAX;
  bounds[3] = SKC_INT_MAX;
}

//
//
//

void
skc_composition_retain_and_lock(struct skc_composition * const composition)
{
  skc_composition_retain(composition);

  composition->impl->lock_count += 1;
}

void
skc_composition_unlock_and_release(struct skc_composition * const composition)
{
  composition->impl->lock_count -= 1;

  skc_composition_pfn_release(composition->impl);
}

//
//
//

skc_err
skc_composition_cl_12_create(struct skc_context       * const context,
                             struct skc_composition * * const composition)
{
  struct skc_runtime * const runtime = context->runtime;

  // retain the context
  // skc_context_retain(context);

  // allocate impl
  struct skc_composition_impl * const impl = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*impl));

  // allocate composition
  (*composition)            = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(**composition));

  (*composition)->context   = context;
  (*composition)->impl      = impl;
  (*composition)->ref_count = 1;

  (*composition)->place     = skc_composition_pfn_place;
  (*composition)->unseal    = skc_composition_pfn_unseal;
  (*composition)->seal      = skc_composition_pfn_seal;
  (*composition)->bounds    = skc_composition_pfn_bounds;
  (*composition)->release   = skc_composition_pfn_release;

  // intialize impl
  impl->composition   = (*composition);
  impl->runtime       = runtime;

  SKC_ASSERT_STATE_INIT(impl,SKC_COMPOSITION_STATE_SEALED);

  impl->lock_count    = 0;

  impl->grids.sort    = NULL;
  impl->grids.place   = NULL;

  // acquire command queue for sealing/unsealing
  impl->cq            = skc_runtime_acquire_cq_in_order(runtime);

  // acquire kernels
  impl->kernels.place   = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_PLACE);
  impl->kernels.segment = skc_device_acquire_kernel(runtime->device, SKC_DEVICE_KERNEL_ID_SEGMENT_TTCK);

  // get config
  struct skc_config const * const config = runtime->config;

  // initialize ring size with config values
  skc_extent_ring_init(&impl->cmds.ring,
                       config->composition.cmds.elem_count,
                       config->composition.cmds.snap_count,
                       sizeof(union skc_cmd_place));

  skc_extent_phw1g_tdrNs_alloc(runtime,&impl->cmds.extent ,sizeof(union skc_cmd_place) * config->composition.cmds.elem_count);
  skc_extent_phrw_alloc       (runtime,&impl->saved.extent,sizeof(skc_raster_t)        * config->composition.raster_ids.elem_count);
  skc_extent_phr_pdrw_alloc   (runtime,&impl->atomics     ,sizeof(struct skc_place_atomics));

  skc_extent_pdrw_alloc       (runtime,&impl->keys        ,sizeof(skc_ttxk_t)          * config->composition.keys.elem_count);
  skc_extent_pdrw_alloc       (runtime,&impl->offsets     ,sizeof(skc_uint)            * (1u << SKC_TTCK_HI_BITS_YX)); // 1MB

  // nothing saved
  impl->saved.count = 0;

  // unseal the composition, zero the atomics, etc.
  skc_composition_unseal_block_reset(impl,false,true);

  return SKC_ERR_SUCCESS;
}

//
//
//
