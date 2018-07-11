/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// squelch OpenCL 1.2 deprecation warning
//

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif

//
//
//

#include <stdio.h>
#include <string.h>

//
//
//

#include "runtime_cl_12.h"
#include "common/cl/assert_cl.h"

//
// This implementation is probably excessive.
//
// The command queue pool could easily be replaced with simply an LRU
// or even round-robin reuse pool.  Even a small number of aliased
// command queues can probably enough concurrency.
//

#define SKC_CQ_POOL_EXPAND 1

//
//
//

static
cl_command_queue
skc_runtime_cl_12_create_cq(struct skc_runtime * const runtime,
                            struct skc_cq_pool * const pool)

{
  cl_command_queue cq;

#if 1
      //
      // <= OpenCL 1.2
      //
      cl_int cl_err;

      cq = clCreateCommandQueue(runtime->cl.context,
                                runtime->cl.device_id,
                                pool->cq_props,
                                &cl_err); cl_ok(cl_err);
#else
  if (runtime_cl->version.major < 2)
    {
      //
      // <= OpenCL 1.2
      //
      cl_int cl_err;

      cq = clCreateCommandQueue(runtime_cl->context,
                                runtime_cl->device_id,
                                (cl_command_queue_properties)type,
                                &cl_err); cl_ok(cl_err);
    }
  else
    {
      //
      // >= OpenCL 2.0
      //
      cl_int                    cl_err;
      cl_queue_properties const queue_properties[] = {
        CL_QUEUE_PROPERTIES,(cl_queue_properties)type,0
      };

      cq = clCreateCommandQueueWithProperties(runtime_cl->context,
                                              runtime_cl->device_id,
                                              queue_properties,
                                              &cl_err); cl_ok(cl_err);
    }
#endif

  return cq;
}

//
//
//

void
skc_cq_pool_create(struct skc_runtime        * const runtime,
                   struct skc_cq_pool        * const pool,
                   cl_command_queue_properties const cq_props,
                   skc_uint                    const size)
{
  pool->size     = size + 1; // an empty spot
  pool->reads    = 0;
  pool->writes   = size;

  pool->cq_props = cq_props;
  pool->cq       = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,
                                               pool->size * sizeof(*pool->cq));
  for (skc_uint ii=0; ii<size; ii++)
    pool->cq[ii] = skc_runtime_cl_12_create_cq(runtime,pool);

  pool->cq[size] = NULL;
}

//
//
//

void
skc_cq_pool_dispose(struct skc_runtime * const runtime,
                    struct skc_cq_pool *       pool)
{
  //
  // FIXME -- release the command queues after waiting for the ring to
  // be full with pool.size queues?
  //
  skc_runtime_host_perm_free(runtime,pool->cq);
}

//
//
//

static
void
skc_cq_pool_write(struct skc_cq_pool * const pool,
                  cl_command_queue           cq)
{
  pool->cq[pool->writes++ % pool->size] = cq;
}

//
// only expand when completely empty
//

static
void
skc_cq_pool_expand(struct skc_runtime * const runtime,
                   struct skc_cq_pool * const pool,
                   skc_uint                   expand)
{
#ifndef NDEBUG
  fprintf(stderr,"Expanding the cq_pool by: %u (%u)\n",expand,pool->size);
#endif

  // free old
  skc_runtime_host_perm_free(runtime,pool->cq);

  // the ring is empty
  pool->size  += expand;
  pool->cq     = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,pool->size * sizeof(*pool->cq));
  pool->reads  = 0;
  pool->writes = expand;

  for (skc_uint ii=0; ii<expand; ii++)
    pool->cq[ii] = skc_runtime_cl_12_create_cq(runtime,pool);
}

//
//
//

static
cl_command_queue
skc_cq_pool_read(struct skc_runtime * const runtime,
                 struct skc_cq_pool * const pool)
{
  // any command queues left?
  if (pool->reads == pool->writes)
    skc_cq_pool_expand(runtime,pool,SKC_CQ_POOL_EXPAND);

  cl_command_queue cq = pool->cq[pool->reads++ % pool->size];

  return cq;
}

//
//
//

cl_command_queue
skc_runtime_acquire_cq_in_order(struct skc_runtime * const runtime)
{
  return skc_cq_pool_read(runtime,&runtime->cq_pool);
}

void
skc_runtime_release_cq_in_order(struct skc_runtime * const runtime,
                                cl_command_queue           cq)
{
  skc_cq_pool_write(&runtime->cq_pool,cq);
}

//
//
//
