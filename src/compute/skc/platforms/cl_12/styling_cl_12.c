/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// NOTES:
//
// - this particular object only needs a command queue for a short
//   time so consider acquiring/releasing the command queue on demand
//   but only if command queues are cached and expensive to keep
//

#include "common/cl/assert_cl.h"

#include "styling_cl_12.h"
#include "extent_cl_12.h"
#include "runtime_cl_12.h"

#include "context.h"
#include "styling_types.h"

//
//
//

static
void
skc_styling_unmap_complete(skc_grid_t const grid)
{
  struct skc_styling_impl * const impl = skc_grid_get_data(grid);

  impl->state = SKC_STYLING_STATE_SEALED;

  skc_grid_complete(grid);
}

static
void
skc_styling_unmap_cb(cl_event event, cl_int status, skc_grid_t const grid)
{
  SKC_CL_CB(status);

  struct skc_styling_impl * const impl      = skc_grid_get_data(grid);
  struct skc_scheduler    * const scheduler = impl->runtime->scheduler;

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(scheduler,skc_styling_unmap_complete,grid);
}

static
void
skc_styling_grid_pfn_execute(skc_grid_t const grid)
{
  struct skc_styling_impl * const impl    = skc_grid_get_data(grid);
  struct skc_styling      * const styling = impl->styling;

  //
  // unmap all extents
  //
  cl_event complete;

  skc_extent_phwN_pdrN_unmap(&impl->layers,styling->layers.extent,impl->cq,NULL);
  skc_extent_phwN_pdrN_unmap(&impl->groups,styling->groups.extent,impl->cq,NULL);
  skc_extent_phwN_pdrN_unmap(&impl->extras,styling->extras.extent,impl->cq,&complete);

  // set the event
  cl(SetEventCallback(complete,CL_COMPLETE,skc_styling_unmap_cb,grid));
  cl(ReleaseEvent(complete));

  // flush command queue
  cl(Flush(impl->cq));
}

//
//
//

static
void
skc_styling_pfn_seal(struct skc_styling_impl * const impl)
{
  // return if sealing or sealed
  if (impl->state >= SKC_STYLING_STATE_SEALING)
    return;

  struct skc_runtime   * const runtime   = impl->runtime;
  struct skc_scheduler * const scheduler = runtime->scheduler;

  //
  // otherwise, wait for UNSEALING > UNSEALED transition
  //
  if (impl->state == SKC_STYLING_STATE_UNSEALING)
    {
      SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_STYLING_STATE_UNSEALED);
    }

  //
  // we're unsealed so we need to seal and start the grid
  //
  impl->state = SKC_STYLING_STATE_SEALING;
  impl->grid  = SKC_GRID_DEPS_ATTACH(runtime->deps,
                                     NULL,
                                     impl,
                                     NULL,  // no waiting
                                     skc_styling_grid_pfn_execute,
                                     NULL); // no dispose

  // no need to force -- styling has no dependencies
  skc_grid_start(impl->grid);
}

//
//
//

void
skc_styling_unseal_complete(struct skc_styling_impl * const impl)
{
  struct skc_runtime * const runtime = impl->runtime;

  // we're now unsealed
  impl->state = SKC_STYLING_STATE_UNSEALED;
}

static
void
skc_styling_unseal_cb(cl_event event, cl_int status, struct skc_styling_impl * const impl)
{
  SKC_CL_CB(status);

  // as quickly as possible, enqueue next stage in pipeline to context command scheduler
  SKC_SCHEDULER_SCHEDULE(impl->runtime->scheduler,skc_styling_unseal_complete,impl);
}

static
void
skc_styling_pfn_unseal(struct skc_styling_impl * const impl, skc_bool const block)
{
  // return if already unsealed
  if (impl->state == SKC_STYLING_STATE_UNSEALED)
    return;

  //
  // otherwise, we're going to need to pump the scheduler
  //
  struct skc_runtime   * const runtime   = impl->runtime;
  struct skc_scheduler * const scheduler = runtime->scheduler;

  //
  // wait for UNSEALING > UNSEALED transition
  //
  if (impl->state == SKC_STYLING_STATE_UNSEALING)
    {
      if (block) {
        SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_STYLING_STATE_UNSEALED);
      }
      return;
    }

  //
  // otherwise, wait for SEALING > SEALED transition ...
  //
  if (impl->state == SKC_STYLING_STATE_SEALING)
    {
      // wait if sealing
      SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_STYLING_STATE_SEALED);
    }

  // wait for rendering locks to be released
  SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->lock_count > 0);

  // ... and then unseal the styling object
  impl->state = SKC_STYLING_STATE_UNSEALING;

  // defensively NULL the grid reference
  impl->grid  = NULL; // defensive

  // set styling pointers with mapped extents
  cl_event complete;

  struct skc_styling * const styling = impl->styling;

  styling->layers.extent = skc_extent_phwN_pdrN_map(&impl->layers,impl->cq,NULL);
  styling->groups.extent = skc_extent_phwN_pdrN_map(&impl->groups,impl->cq,NULL);
  styling->extras.extent = skc_extent_phwN_pdrN_map(&impl->extras,impl->cq,&complete);

  cl(SetEventCallback(complete,CL_COMPLETE,skc_styling_unseal_cb,impl));
  cl(ReleaseEvent(complete));

  // flush it
  cl(Flush(impl->cq));

  // wait until unsealed...
  if (block) {
    SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_STYLING_STATE_UNSEALED);
  }
}

//
//
//

static
void
skc_styling_pfn_release(struct skc_styling_impl * const impl)
{
  if (--impl->styling->ref_count != 0)
    return;

  //
  // otherwise, unmap all resources
  //

  //
  // FIXME -- is it pointless unmap before freeing?  The seal
  // accomplishes the unmapping.
  //
  skc_styling_pfn_seal(impl);

  struct skc_runtime   * const runtime   = impl->runtime;
  struct skc_scheduler * const scheduler = runtime->scheduler;

  // wait until sealed
  SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->state != SKC_STYLING_STATE_SEALED);

  // wait for locks to drain
  SKC_SCHEDULER_WAIT_WHILE(scheduler,impl->lock_count > 0)

  //
  // styling is now disposable
  //

  // free styling host
  skc_runtime_host_perm_free(runtime,impl->styling);

  // release the cq
  skc_runtime_release_cq_in_order(runtime,impl->cq);

  // free extents
  skc_extent_phwN_pdrN_free(runtime,&impl->layers);
  skc_extent_phwN_pdrN_free(runtime,&impl->groups);
  skc_extent_phwN_pdrN_free(runtime,&impl->extras);

  // free styling impl
  skc_runtime_host_perm_free(runtime,impl);
}

//
//
//

void
skc_styling_retain_and_lock(struct skc_styling * const styling)
{
  skc_styling_retain(styling);

  styling->impl->lock_count += 1;
}

void
skc_styling_unlock_and_release(struct skc_styling * const styling)
{
  styling->impl->lock_count -= 1;

  skc_styling_pfn_release(styling->impl);
}

//
//
//

skc_err
skc_styling_cl_12_create(struct skc_context   * const context,
                         struct skc_styling * * const styling,
                         skc_uint               const layers_count,
                         skc_uint               const groups_count,
                         skc_uint               const extras_count)
{
  // retain the context
  // skc_context_retain(context);

  // allocate the impl
  struct skc_runtime      * const runtime = context->runtime;
  struct skc_styling_impl * const impl    = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*impl));

  // allocate styling
  (*styling)          = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(**styling));
  (*styling)->context = context;
  (*styling)->impl    = impl;

  // intialize impl
  impl->styling       = (*styling);
  impl->runtime       = runtime;

  SKC_ASSERT_STATE_INIT(impl,SKC_STYLING_STATE_SEALED);

  impl->lock_count    = 0;

  impl->cq            = skc_runtime_acquire_cq_in_order(runtime);

  //
  // The styling object is unique in that the API lets the user
  // specify resource limits
  //
  // The styling object is a simple container that can have wildly
  // varying resource requirements (but still relatively modest).
  //
  // Additionally, an advanced SKC programmer may want to create many
  // styling and composition objects as they're relatively cheap.
  //
  skc_extent_phwN_pdrN_alloc(runtime,&impl->layers,sizeof(*(*styling)->layers.extent) * layers_count);
  skc_extent_phwN_pdrN_alloc(runtime,&impl->groups,sizeof(*(*styling)->groups.extent) * groups_count);
  skc_extent_phwN_pdrN_alloc(runtime,&impl->extras,sizeof(*(*styling)->extras.extent) * extras_count);

  // initialize styling
  (*styling)->layers.size  = layers_count;
  (*styling)->groups.size  = groups_count;
  (*styling)->extras.size  = extras_count;

  (*styling)->layers.count = 0;
  (*styling)->groups.count = 0;
  (*styling)->extras.count = 0;

  // save pfns
  (*styling)->seal         = skc_styling_pfn_seal;
  (*styling)->unseal       = skc_styling_pfn_unseal;
  (*styling)->release      = skc_styling_pfn_release;

  // set ref count
  (*styling)->ref_count    = 1;

  // map the extents by unsealing
  skc_styling_pfn_unseal(impl,false);

  return SKC_ERR_SUCCESS;
}

//
//
//
