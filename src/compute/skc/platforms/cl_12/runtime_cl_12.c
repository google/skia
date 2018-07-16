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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//
//
//

#include "context.h"
#include "block.h"
#include "grid.h"
#include "common/cl/assert_cl.h"
#include "config_cl.h"
#include "runtime_cl_12.h"
#include "export_cl_12.h"

//
//
//

static
void
skc_block_pool_create(struct skc_runtime * const runtime, cl_command_queue cq)
{
  // save size
  runtime->block_pool.size = &runtime->config->block_pool;

  // create block extent
  skc_extent_pdrw_alloc(runtime,
                        &runtime->block_pool.blocks,
                        runtime->block_pool.size->pool_size *
                        runtime->config->block.bytes);

  // allocate block pool ids
  skc_extent_pdrw_alloc(runtime,
                        &runtime->block_pool.ids,
                        runtime->block_pool.size->ring_pow2 * sizeof(skc_uint));

  // allocate block pool atomics
  skc_extent_phr_pdrw_alloc(runtime,
                            &runtime->block_pool.atomics,
                            sizeof(union skc_block_pool_atomic));

  // acquire pool id and atomic initialization kernels
  cl_kernel k0 = skc_device_acquire_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_IDS);
  cl_kernel k1 = skc_device_acquire_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_ATOMICS);

  // init ids
  cl(SetKernelArg(k0,0,sizeof(runtime->block_pool.ids.drw),&runtime->block_pool.ids.drw));
  cl(SetKernelArg(k0,1,SKC_CL_ARG(runtime->block_pool.size->pool_size)));

  // the kernel grid is shaped by the target device -- always 2 for atomics
  skc_device_enqueue_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_IDS,
                            cq,k0,runtime->block_pool.size->pool_size,
                            0,NULL,NULL);

  // init atomics
  cl(SetKernelArg(k1,0,sizeof(runtime->block_pool.atomics.drw),&runtime->block_pool.atomics.drw));
  cl(SetKernelArg(k1,1,SKC_CL_ARG(runtime->block_pool.size->pool_size)));

  // the kernel grid is shaped by the target device
  skc_device_enqueue_kernel(runtime->device,SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_ATOMICS,
                            cq,k1,2,
                            0,NULL,NULL);

  // kickstart kernel execution
  cl(Flush(cq));

  // release kernels
  cl(ReleaseKernel(k0));
  cl(ReleaseKernel(k1));
}

static
void
skc_block_pool_dispose(struct skc_runtime * const runtime)
{
  skc_extent_phr_pdrw_free(runtime,&runtime->block_pool.atomics);
  skc_extent_pdrw_free    (runtime,&runtime->block_pool.ids);
  skc_extent_pdrw_free    (runtime,&runtime->block_pool.blocks);
}

//
//
//

static
bool
skc_runtime_yield(struct skc_runtime * const runtime)
{
  return skc_scheduler_yield(runtime->scheduler);
}

static
void
skc_runtime_wait(struct skc_runtime * const runtime)
{
  skc_scheduler_wait(runtime->scheduler);
}

//
//
//

skc_err
skc_runtime_cl_12_create(struct skc_context * const context,
                         cl_context                 context_cl,
                         cl_device_id               device_id_cl)
{
  // allocate the runtime
  struct skc_runtime * const runtime = malloc(sizeof(*runtime));

  // save off CL objects
  runtime->cl.context   = context_cl;
  runtime->cl.device_id = device_id_cl;

  // query device alignment
  cl_uint align_bits;

  cl(GetDeviceInfo(device_id_cl,
                   CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                   sizeof(align_bits),
                   &align_bits,
                   NULL));

  runtime->cl.align_bytes = align_bits / 8;

  // create device
  skc_device_create(runtime);

  // create the host and device allocators
  skc_allocator_host_create(runtime);
  skc_allocator_device_create(runtime);

  // how many slots in the scheduler?
  runtime->scheduler = skc_scheduler_create(runtime,runtime->config->scheduler.size);

  // allocate deps structure
  runtime->deps      = skc_grid_deps_create(runtime,
                                            runtime->scheduler,
                                            runtime->config->block_pool.pool_size);

  // initialize cq pool
  skc_cq_pool_create(runtime,
                     &runtime->cq_pool,
                     runtime->config->cq_pool.cq_props,
                     runtime->config->cq_pool.size);

  // acquire in-order cq
  cl_command_queue cq = skc_runtime_acquire_cq_in_order(runtime);

  // initialize block pool
  skc_block_pool_create(runtime,cq);

  // intialize handle pool
  skc_handle_pool_create(runtime,
                         &runtime->handle_pool,
                         runtime->config->handle_pool.size,
                         runtime->config->handle_pool.width,
                         runtime->config->handle_pool.recs);

  //
  // initialize pfns
  //
  // FIXME -- at this point we will have identified which device we've
  // targeted and will load a DLL (or select from a built-in library)
  // that contains all the pfns.
  //
  context->runtime        = runtime;

  context->yield          = skc_runtime_yield;
  context->wait           = skc_runtime_wait;

  context->path_builder   = skc_path_builder_cl_12_create;
  context->path_retain    = skc_runtime_path_host_retain;
  context->path_release   = skc_runtime_path_host_release;
  context->path_flush     = skc_runtime_path_host_flush;

  context->raster_builder = skc_raster_builder_cl_12_create;
  context->raster_retain  = skc_runtime_raster_host_retain;
  context->raster_release = skc_runtime_raster_host_release;
  context->raster_flush   = skc_runtime_raster_host_flush;

  context->composition    = skc_composition_cl_12_create;
  context->styling        = skc_styling_cl_12_create;

  context->surface        = skc_surface_cl_12_create;

  // block on pool creation
  cl(Finish(cq));

  // dispose of in-order cq
  skc_runtime_release_cq_in_order(runtime,cq);

  return SKC_ERR_SUCCESS;
};

//
//
//

skc_err
skc_runtime_cl_12_dispose(struct skc_context * const context)
{
  //
  // FIXME -- incomplete
  //
  fprintf(stderr,"%s incomplete!\n",__func__);

  struct skc_runtime * runtime = context->runtime;

  skc_allocator_device_dispose(runtime);
  skc_allocator_host_dispose(runtime);

  skc_scheduler_dispose(context->runtime,context->runtime->scheduler);

  skc_grid_deps_dispose(context->runtime->deps);

  skc_cq_pool_dispose(runtime,&runtime->cq_pool);

  skc_block_pool_dispose(context->runtime);

  // skc_handle_pool_dispose(context->runtime);

  return SKC_ERR_SUCCESS;
}

//
// REPORT BLOCK POOL ALLOCATION
//

void
skc_runtime_cl_12_debug(struct skc_context * const context)
{
  struct skc_runtime * const runtime = context->runtime;

  // acquire out-of-order cq
  cl_command_queue cq = skc_runtime_acquire_cq_in_order(runtime);

  // copy atomics to host
  skc_extent_phr_pdrw_read(&runtime->block_pool.atomics,cq,NULL);

  // block until complete
  cl(Finish(cq));

  // dispose of out-of-order cq
  skc_runtime_release_cq_in_order(runtime,cq);

  union skc_block_pool_atomic const * const bp_atomic = runtime->block_pool.atomics.hr;

  skc_uint const available = bp_atomic->writes - bp_atomic->reads;
  skc_uint const inuse     = runtime->config->block_pool.pool_size - available;

  fprintf(stderr,
          "writes/reads/avail/alloc: %9u / %9u / %9u = %6.2f MB / %9u = %6.2f MB\n",
          bp_atomic->writes,
          bp_atomic->reads,
          available,
          (available * runtime->config->block.bytes) / (1024.0*1024.0),
          inuse,
          (inuse     * runtime->config->block.bytes) / (1024.0*1024.0));
}

//
//
//
