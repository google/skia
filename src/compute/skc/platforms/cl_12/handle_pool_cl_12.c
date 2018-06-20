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
#include <assert.h>

//
//
//

#include "common/cl/assert_cl.h"

#include "block.h"
#include "grid.h"
#include "config_cl.h"
#include "runtime_cl_12.h"

//
// FIXME -- these comments are now quite stale
//
//
// HANDLE/ACQUIRE RELEASE
//
// The runtime vends handles just in case we decide to exploit shared
// virtual memory.  But for most platforms and devices we will have a
// pool of host-managed handles and on the device there will be a
// table that maps the host handle to a device-managed memory block.
//
// HANDLE READINESS
//
// A host handle may reference a path or a raster which is not ready
// for use further down the pipeline because it hasn't yet been
// processed by the device.
//
// The simplest scheme for providing every handle a readiness state is
// to build a map that that marks a new handle as being not-ready
// while being processed by a particular grid id.  When the final
// sub-pipeline grid responsible for the path or raster is complete,
// then mark the handle as being ready and eventually return the grid
// id back to the pool.  This can be performed on a separate thread.
//
// The side-benefit of this approach is that a handle's reference
// count integral type can spare some bits for its associated grid id.
//
// A more memory-intensive approach uses a 64-bit epoch+grid key and
// relies on the ~56 bits of epoch space to avoid any post
// sub-pipeline status update by assuming that a handle and grid will
// match or mismatch when queried.
//

#define SKC_HANDLE_REFCNT_HOST_BITS   (SKC_MEMBER_SIZE(union skc_handle_refcnt,h) * 8)
#define SKC_HANDLE_REFCNT_DEVICE_BITS (SKC_MEMBER_SIZE(union skc_handle_refcnt,d) * 8)

#define SKC_HANDLE_REFCNT_HOST_MAX    SKC_BITS_TO_MASK(SKC_HANDLE_REFCNT_HOST_BITS)
#define SKC_HANDLE_REFCNT_DEVICE_MAX  SKC_BITS_TO_MASK(SKC_HANDLE_REFCNT_DEVICE_BITS)

//
//
//

static
void
skc_handle_reclaim_create(struct skc_runtime      * const runtime,
                          struct skc_handle_pool  * const handle_pool,
                          skc_handle_reclaim_type_e const reclaim_type,
                          skc_device_kernel_id      const kernel_id)
{
  struct skc_handle_reclaim * const reclaim = handle_pool->reclaim + reclaim_type;

  // init counters
  reclaim->bih.rem   = 0;

  // acquire kernel
  reclaim->kernel    = skc_device_acquire_kernel(runtime->device,kernel_id);
  reclaim->kernel_id = kernel_id;

  // set default args
  cl(SetKernelArg(reclaim->kernel,0,SKC_CL_ARG(runtime->block_pool.ids.drw)));
  cl(SetKernelArg(reclaim->kernel,1,SKC_CL_ARG(runtime->block_pool.blocks.drw)));
  cl(SetKernelArg(reclaim->kernel,2,SKC_CL_ARG(runtime->block_pool.atomics.drw)));
  cl(SetKernelArg(reclaim->kernel,3,SKC_CL_ARG(runtime->config->block_pool.ring_mask)));
  cl(SetKernelArg(reclaim->kernel,4,SKC_CL_ARG(runtime->handle_pool.map.drw)));
}

static
void
skc_handle_reclaim_dispose(struct skc_runtime      * const runtime,
                           skc_handle_reclaim_type_e const reclaim_type)
{
  struct skc_handle_reclaim * const reclaim = runtime->handle_pool.reclaim + reclaim_type;

  cl(ReleaseKernel(reclaim->kernel));
}

//
//
//

#define SKC_HANDLE_POOL_BLOCKS_PAD  8

void
skc_handle_pool_create(struct skc_runtime     * const runtime,
                       struct skc_handle_pool * const handle_pool,
                       skc_uint                 const size,
                       skc_uint                 const width,
                       skc_uint                 const recs)
{
  skc_uint const blocks         = (size + width - 1) / width;
  skc_uint const blocks_padded  = blocks + SKC_HANDLE_POOL_BLOCKS_PAD;
  skc_uint const handles        = blocks        * width;
  skc_uint const handles_padded = blocks_padded * width;
  skc_uint const recs_padded    = recs + 2; // one for pointer and one for head node

  skc_extent_pdrw_alloc(runtime,&handle_pool->map,handles * sizeof(skc_block_id_t));

  handle_pool->handle.indices   = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,handles_padded * sizeof(*handle_pool->handle.indices));
  handle_pool->handle.refcnts   = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,handles        * sizeof(*handle_pool->handle.refcnts));
  handle_pool->block.indices    = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,blocks_padded  * sizeof(*handle_pool->block.indices));
  handle_pool->recs             = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,recs_padded    * sizeof(*handle_pool->recs));

  // initialize handles and refcnts
  for (skc_uint ii=0; ii<handles; ii++)
    handle_pool->handle.indices[ii] = ii;

  for (skc_uint ii=0; ii<handles; ii++)
    handle_pool->handle.refcnts[ii].hd = 0;

  handle_pool->handle.count     = handles;

  // initialize block accounting
  for (skc_uint ii=0; ii<blocks_padded; ii++)
    handle_pool->block.indices[ii] = ii;

  handle_pool->block.count      = blocks_padded;
  handle_pool->block.width      = width;

  handle_pool->block.tos        = blocks; // pop = pre-decrement  / push = post-increment
  handle_pool->block.bos        = blocks; // pop = post-increment / push = pre-decrement

  // initialize recs -- first two elements are interpreted differently
  handle_pool->recs[0].runtime  = runtime;
  handle_pool->recs[1]          = (union skc_handle_reclaim_rec){ .rem = recs, .head = 2 };

  for (skc_uint ii=2; ii<recs_padded; ii++)
    handle_pool->recs[ii] = (union skc_handle_reclaim_rec){ .index = ii, .next = ii+1 };

  handle_pool->recs[recs_padded-1].next = SKC_UINT_MAX;

  // initialize acquire
  handle_pool->acquire.rem = 0;

  // create reclaimers
  skc_handle_reclaim_create(runtime,
                            handle_pool,
                            SKC_HANDLE_RECLAIM_TYPE_PATH,
                            SKC_DEVICE_KERNEL_ID_PATHS_RECLAIM);

  skc_handle_reclaim_create(runtime,
                            handle_pool,
                            SKC_HANDLE_RECLAIM_TYPE_RASTER,
                            SKC_DEVICE_KERNEL_ID_RASTERS_RECLAIM);
}

//
//
//

void
skc_handle_pool_dispose(struct skc_runtime     * const runtime,
                        struct skc_handle_pool * const handle_pool)
{
  skc_handle_reclaim_dispose(runtime,SKC_HANDLE_RECLAIM_TYPE_RASTER);
  skc_handle_reclaim_dispose(runtime,SKC_HANDLE_RECLAIM_TYPE_PATH);

  skc_runtime_host_perm_free(runtime,handle_pool->recs);
  skc_runtime_host_perm_free(runtime,handle_pool->block.indices);
  skc_runtime_host_perm_free(runtime,handle_pool->handle.refcnts);
  skc_runtime_host_perm_free(runtime,handle_pool->handle.indices);

  skc_extent_pdrw_free(runtime,&handle_pool->map);
}

//
//
//

static
skc_uint
skc_handle_pool_block_readable_pop(struct skc_runtime     * const runtime,
                                   struct skc_handle_pool * const handle_pool)
{
  SKC_SCHEDULER_WAIT_WHILE(runtime->scheduler,handle_pool->block.tos == 0);

  skc_uint const index = handle_pool->block.indices[--handle_pool->block.tos];

#if 0
  skc_handle_t * handles = handle_pool->handle.indices + (index + 1) * handle_pool->block.width;
  for (skc_uint ii=0; ii<handle_pool->block.width; ii++)
    printf("R-: %u\n",*--handles);
#endif

  return index;
}

static
void
skc_handle_pool_block_readable_push(struct skc_handle_pool * const handle_pool,
                                    skc_uint                 const index)
{
  handle_pool->block.indices[handle_pool->block.tos++] = index;

#if 0
  skc_handle_t * handles = handle_pool->handle.indices + (index + 1) * handle_pool->block.width;
  for (skc_uint ii=0; ii<handle_pool->block.width; ii++)
    printf("R+: %u\n",*--handles);
#endif
}


static
skc_uint
skc_handle_pool_block_writable_pop(struct skc_runtime     * const runtime,
                                   struct skc_handle_pool * const handle_pool)
{
  SKC_SCHEDULER_WAIT_WHILE(runtime->scheduler,handle_pool->block.bos == handle_pool->block.count);

  return handle_pool->block.indices[handle_pool->block.bos++];
}

static
void
skc_handle_pool_block_writable_push(struct skc_handle_pool * const handle_pool,
                                    skc_uint                 const block_idx)
{
  handle_pool->block.indices[--handle_pool->block.bos] = block_idx;
}

//
// May need to acquire the path or raster handle *early* just to be
// sure one exists
//

skc_handle_t
skc_runtime_handle_device_acquire(struct skc_runtime * const runtime)
{
  struct skc_handle_pool * const handle_pool = &runtime->handle_pool;

  // acquire a block of handles at a time
  if (handle_pool->acquire.rem == 0)
    {
      skc_uint const block_idx = skc_handle_pool_block_readable_pop(runtime,handle_pool);

      handle_pool->acquire.block   = block_idx;
      handle_pool->acquire.rem     = handle_pool->block.width;
      handle_pool->acquire.handles = handle_pool->handle.indices + (block_idx + 1) * handle_pool->block.width;
    }

  // load handle from next block slot
  skc_uint     const rem    =  --handle_pool->acquire.rem;
  skc_handle_t const handle = *--handle_pool->acquire.handles;

  // initialize refcnt for handle
  handle_pool->handle.refcnts[handle] = (union skc_handle_refcnt){ .h = 1, .d = 1 };

  // if this was the last handle in the block then move the block id
  // to the reclamation stack to be used as a scratchpad
  if (rem == 0) {
    skc_handle_pool_block_writable_push(handle_pool,handle_pool->acquire.block);
  }

  return handle;
}

//
//
//

static
void
skc_handle_reclaim_completion(union skc_handle_reclaim_rec * const recN)
{
  // get root rec which contains pointer to runtime
  union skc_handle_reclaim_rec * const rec0 = recN - recN->index;
  union skc_handle_reclaim_rec * const rec1 = rec0 + 1;

  // return block for reading
  skc_handle_pool_block_readable_push(&rec0->runtime->handle_pool,recN->block);

  // recN is new head of list
  recN->next = rec1->head;
  rec1->head = recN->index;
  rec1->rem += 1;
}

static
void
skc_handle_reclaim_cb(cl_event event, cl_int status, union skc_handle_reclaim_rec * const recN)
{
  SKC_CL_CB(status);

  union skc_handle_reclaim_rec * const rec0 = recN - recN->index;

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(rec0->runtime->scheduler,skc_handle_reclaim_completion,recN);
}

//
// FIXME -- is there an issue launching on the host thread?
//

static
void
skc_handle_reclaim_launch(struct skc_runtime            * const runtime,
                          struct skc_handle_pool        * const handle_pool,
                          struct skc_handle_reclaim     * const reclaim,
                          union  skc_handle_reclaim_rec * const recN)
{
  cl(SetKernelArg(reclaim->kernel,
                  5,
                  handle_pool->block.width * sizeof(skc_handle_t),
                  reclaim->bih.handles));

  // acquire a cq
  cl_command_queue cq = skc_runtime_acquire_cq_in_order(runtime);

  cl_event complete;

  // the kernel grid is shaped by the target device
  skc_device_enqueue_kernel(runtime->device,
                            reclaim->kernel_id,
                            cq,
                            reclaim->kernel,
                            handle_pool->block.width,
                            0,NULL,&complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_handle_reclaim_cb,recN));
  cl(ReleaseEvent(complete));

  // kickstart kernel execution
  cl(Flush(cq));

  // release the cq
  skc_runtime_release_cq_in_order(runtime,cq);
}

//
// reclaim a handle
//

static
union skc_handle_reclaim_rec *
skc_handle_acquire_reclaim_rec(struct skc_runtime     * const runtime,
                               struct skc_handle_pool * const handle_pool)
{
  union skc_handle_reclaim_rec * const rec1 = handle_pool->recs + 1;

  SKC_SCHEDULER_WAIT_WHILE(runtime->scheduler,rec1->rem == 0);

  union skc_handle_reclaim_rec * const recN = handle_pool->recs + rec1->head;

  rec1->head = recN->next;
  rec1->rem -= 1;

  // fprintf(stderr,"rec1->rem = %u\n",rec1->rem);

  return recN;
}

static
void
skc_runtime_device_reclaim(struct skc_runtime        * const runtime,
                           struct skc_handle_pool    * const handle_pool,
                           struct skc_handle_reclaim * const reclaim,
                           skc_handle_t                const handle)
{
  // grab a new block?
  if (reclaim->bih.rem == 0)
    {
      skc_uint const block_idx = skc_handle_pool_block_writable_pop(runtime,handle_pool);

      reclaim->bih.block   = block_idx;
      reclaim->bih.rem     = handle_pool->block.width;
      reclaim->bih.handles = handle_pool->handle.indices + (block_idx + 1) * handle_pool->block.width;
    }

  // store handle -- handle's refcnt was already set to {0:0}
  *--reclaim->bih.handles = handle;

  // if block is full then launch reclamation kernel
  if (--reclaim->bih.rem == 0)
    {
      union skc_handle_reclaim_rec * recN = skc_handle_acquire_reclaim_rec(runtime,handle_pool);

      recN->block = reclaim->bih.block;

      skc_handle_reclaim_launch(runtime,handle_pool,reclaim,recN);
    }
}

//
// Validate host-provided handles before retaining.
//
// Retain validation consists of:
//
//   - correct handle type
//   - handle is in range of pool
//   - host refcnt is not zero
//   - host refcnt is not at the maximum value
//
// After validation, retain the handles for the host
//

static
skc_err
skc_runtime_handle_host_validated_retain(struct skc_runtime       * const runtime,
                                         skc_typed_handle_type_e    const handle_type,
                                         skc_typed_handle_t const * const typed_handles,
                                         uint32_t                   const count)
{
  //
  // FIXME -- test to make sure handles aren't completely out of range integers
  //

  union skc_handle_refcnt * const refcnts = runtime->handle_pool.handle.refcnts;

  for (skc_uint ii=0; ii<count; ii++)
    {
      skc_typed_handle_t const typed_handle = typed_handles[ii];

      if (!SKC_TYPED_HANDLE_IS_TYPE(typed_handle,handle_type))
        {
          return SKC_ERR_HANDLE_INVALID;
        }
      else
        {
          skc_handle_t const handle = SKC_TYPED_HANDLE_TO_HANDLE(typed_handle);

          if (handle >= runtime->handle_pool.handle.count)
            {
              return SKC_ERR_HANDLE_INVALID;
            }
          else
            {
              union skc_handle_refcnt * const refcnt_ptr = refcnts + handle;
              skc_uint                  const host       = refcnt_ptr->h;

              if (host == 0)
                {
                  return SKC_ERR_HANDLE_INVALID;
                }
              else if (host == SKC_HANDLE_REFCNT_HOST_MAX)
                {
                  return SKC_ERR_HANDLE_OVERFLOW;
                }
            }
        }
    }

  //
  // all the handles validated, so retain them all..
  //
  for (skc_uint ii=0; ii<count; ii++)
    refcnts[SKC_TYPED_HANDLE_TO_HANDLE(typed_handles[ii])].h++;

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_runtime_path_host_retain(struct skc_runtime * const runtime,
                             skc_path_t   const *       paths,
                             uint32_t                   count)
{
  return skc_runtime_handle_host_validated_retain(runtime,
                                                  SKC_TYPED_HANDLE_TYPE_IS_PATH,
                                                  paths,
                                                  count);
}

skc_err
skc_runtime_raster_host_retain(struct skc_runtime * const runtime,
                               skc_path_t   const *       rasters,
                               uint32_t                   count)
{
  return skc_runtime_handle_host_validated_retain(runtime,
                                                  SKC_TYPED_HANDLE_TYPE_IS_RASTER,
                                                  rasters,
                                                  count);
}

//
//
//

skc_err
skc_runtime_raster_host_flush(struct skc_runtime * const runtime,
                               skc_raster_t const *       rasters,
                               uint32_t                   count)
{
  skc_grid_deps_force(runtime->deps,rasters,count);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_runtime_path_host_flush(struct skc_runtime * const runtime,
                            skc_path_t   const *       paths,
                            uint32_t                   count)
{
  skc_grid_deps_force(runtime->deps,paths,count);

  return SKC_ERR_SUCCESS;
}

//
// Validate host-provided handles before releasing.
//
// Release validation consists of:
//
//   - correct handle type
//   - handle is in range of pool
//   - host refcnt is not zero
//
// After validation, release the handles for the host
//

static
skc_err
skc_runtime_host_validated_release(struct skc_runtime       * const runtime,
                                   skc_typed_handle_type_e    const type,
                                   skc_handle_reclaim_type_e  const reclaim_type,
                                   skc_typed_handle_t const * const handles,
                                   uint32_t                   const count)
{
  struct skc_handle_pool   * const handle_pool = &runtime->handle_pool;
  union  skc_handle_refcnt * const refcnts     = handle_pool->handle.refcnts;

  for (skc_uint ii=0; ii<count; ii++)
    {
      skc_typed_handle_t const typed_handle = handles[ii];

      if (!SKC_TYPED_HANDLE_IS_TYPE(typed_handle,type))
        {
          return SKC_ERR_HANDLE_INVALID;
        }
      else
        {
          skc_handle_t const handle = SKC_TYPED_HANDLE_TO_HANDLE(typed_handle);

          if (handle >= handle_pool->handle.count)
            {
              return SKC_ERR_HANDLE_INVALID;
            }
          else
            {
              union skc_handle_refcnt * const refcnt_ptr = refcnts + handle;
              skc_uint                  const host       = refcnt_ptr->h;

              if (host == 0)
                {
                  return SKC_ERR_HANDLE_INVALID;
                }
            }
        }
    }

  //
  // all the handles validated, so release them all..
  //
  struct skc_handle_reclaim * const reclaim = handle_pool->reclaim + reclaim_type;

  for (skc_uint ii=0; ii<count; ii++)
    {
      skc_handle_t              const handle     = SKC_TYPED_HANDLE_TO_HANDLE(handles[ii]);
      union skc_handle_refcnt * const refcnt_ptr = refcnts + handle;
      union skc_handle_refcnt         refcnt     = *refcnt_ptr;

      refcnt.h   -= 1;
      *refcnt_ptr = refcnt;

      if (refcnt.hd == 0) {
        skc_runtime_device_reclaim(runtime,handle_pool,reclaim,handle);
      }
    }

  return SKC_ERR_SUCCESS;
}

//
//
//

skc_err
skc_runtime_path_host_release(struct skc_runtime * const runtime,
                              skc_path_t   const *       paths,
                              uint32_t                   count)
{
  return skc_runtime_host_validated_release(runtime,
                                            SKC_TYPED_HANDLE_TYPE_IS_PATH,
                                            SKC_HANDLE_RECLAIM_TYPE_PATH,
                                            paths,
                                            count);
}

skc_err
skc_runtime_raster_host_release(struct skc_runtime * const runtime,
                                skc_raster_t const *       rasters,
                                uint32_t                   count)
{
  return skc_runtime_host_validated_release(runtime,
                                            SKC_TYPED_HANDLE_TYPE_IS_RASTER,
                                            SKC_HANDLE_RECLAIM_TYPE_RASTER,
                                            rasters,
                                            count);
}

//
// Validate host-provided handles before retaining on the device.
//
//   - correct handle type
//   - handle is in range of pool
//   - host refcnt is not zero
//   - device refcnt is not at the maximum value
//

skc_err
skc_runtime_handle_device_validate_retain(struct skc_runtime       * const runtime,
                                          skc_typed_handle_type_e    const type,
                                          skc_typed_handle_t const *       handles,
                                          uint32_t                         count)
{
  union skc_handle_refcnt * const refcnts = runtime->handle_pool.handle.refcnts;

  while (count-- > 0)
    {
      skc_typed_handle_t const typed_handle = *handles++;

      if (!SKC_TYPED_HANDLE_IS_TYPE(typed_handle,type))
        {
          return SKC_ERR_HANDLE_INVALID;
        }
      else
        {
          skc_handle_t const handle = SKC_TYPED_HANDLE_TO_HANDLE(typed_handle);

          if (handle >= runtime->handle_pool.handle.count)
            {
              return SKC_ERR_HANDLE_INVALID;
            }
          else
            {
              union skc_handle_refcnt * const refcnt_ptr = refcnts + handle;
              union skc_handle_refcnt         refcnt     = *refcnt_ptr;

              if (refcnt.h == 0)
                {
                  return SKC_ERR_HANDLE_INVALID;
                }
              else if (refcnt.d == SKC_HANDLE_REFCNT_DEVICE_MAX)
                {
                  return SKC_ERR_HANDLE_OVERFLOW;
                }
            }
        }
    }

  return SKC_ERR_SUCCESS;
}

//
// After validation, retain the handles for the device
//

void
skc_runtime_handle_device_retain(struct skc_runtime * const runtime,
                                 skc_handle_t const *       handles,
                                 uint32_t                   count)
{
  union skc_handle_refcnt * const refcnts = runtime->handle_pool.handle.refcnts;

  while (count-- > 0)
    refcnts[SKC_TYPED_HANDLE_TO_HANDLE(*handles++)].d++;
}

//
// Release the device-held handles -- no validation required!
//

static
void
skc_runtime_handle_device_release(struct skc_runtime      * const runtime,
                                  skc_handle_reclaim_type_e const reclaim_type,
                                  skc_handle_t      const *       handles,
                                  skc_uint                        count)
{
  struct skc_handle_pool    * const handle_pool = &runtime->handle_pool;
  union  skc_handle_refcnt  * const refcnts     = handle_pool->handle.refcnts;
  struct skc_handle_reclaim * const reclaim     = handle_pool->reclaim + reclaim_type;

  while (count-- > 0) {
    skc_handle_t              const handle     = *handles++;
    union skc_handle_refcnt * const refcnt_ptr = refcnts + handle;
    union skc_handle_refcnt         refcnt     = *refcnt_ptr;

    refcnt.d   -= 1;
    *refcnt_ptr = refcnt;

#if 0
    printf("%8u = { %u, %u }\n",handle,refcnt.h,refcnt.d);
#endif

    if (refcnt.hd == 0) {
      skc_runtime_device_reclaim(runtime,handle_pool,reclaim,handle);
    }
  }
}

//
//
//

void
skc_runtime_path_device_release(struct skc_runtime * const runtime,
                                skc_handle_t const *       handles,
                                skc_uint                   count)
{
  skc_runtime_handle_device_release(runtime,SKC_HANDLE_RECLAIM_TYPE_PATH,handles,count);
}

void
skc_runtime_raster_device_release(struct skc_runtime * const runtime,
                                  skc_handle_t const *       handles,
                                  skc_uint                   count)
{
  skc_runtime_handle_device_release(runtime,SKC_HANDLE_RECLAIM_TYPE_RASTER,handles,count);
}

//
//
//
