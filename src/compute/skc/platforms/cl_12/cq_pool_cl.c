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

#ifndef NDEBUG
#include <stdio.h>
#endif

//
//
//

#include <string.h>

//
//
//

#include "runtime_cl_12.h"

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

void
skc_cq_pool_create(struct skc_runtime * const runtime,
                   struct skc_cq_pool * const pool,
                   skc_uint             const type,
                   skc_uint             const size)
{
  pool->type   = type;
  pool->size   = size + 1; // an empty spot
  pool->reads  = 0;
  pool->writes = size;
  pool->cq     = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,pool->size * sizeof(*pool->cq));

  for (skc_uint ii=0; ii<size; ii++) {
    pool->cq[ii] = skc_runtime_cl_create_cq(&runtime->cl,pool->type);
  }
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
    pool->cq[ii] = skc_runtime_cl_create_cq(&runtime->cl,pool->type);
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
