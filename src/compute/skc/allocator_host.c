/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <stdlib.h>

//
//
//

#include "runtime_cl_12.h"
#include "config_cl.h"

//
//
//

#define SKC_RUNTIME_HOST_CACHELINE_SIZE  64

#define SKC_ALIGNED_MALLOC(size,alignment) _aligned_malloc(size,alignment)
#define SKC_ALIGNED_FREE(p)                _aligned_free(p)

//
// PERM
//

void *
skc_runtime_host_perm_alloc(struct skc_runtime * const runtime,
                            skc_mem_flags_e      const flags,
                            size_t               const size)
{
  return SKC_ALIGNED_MALLOC(SKC_ROUND_UP(size,SKC_RUNTIME_HOST_CACHELINE_SIZE),
                            SKC_RUNTIME_HOST_CACHELINE_SIZE);
}

void
skc_runtime_host_perm_free(struct skc_runtime * const runtime,
                           void               * const mem)
{
  SKC_ALIGNED_FREE(mem);
}

//
// TEMP
//

void *
skc_runtime_host_temp_alloc(struct skc_runtime * const runtime,
                            skc_mem_flags_e      const flags,
                            size_t               const size,
                            skc_subbuf_id_t    * const subbuf_id,
                            size_t             * const subbuf_size)
{
  if (size == 0)
    {
      *subbuf_id = (skc_subbuf_id_t)-1;

      if (subbuf_size != NULL)
        *subbuf_size = 0;

      return NULL;
    }

  return runtime->allocator.host.temp.extent +
    skc_suballocator_subbuf_alloc(&runtime->allocator.host.temp.suballocator,
                                  runtime->scheduler,
                                  size,subbuf_id,subbuf_size);
}


void
skc_runtime_host_temp_free(struct skc_runtime * const runtime,
                           void               * const mem,
                           skc_subbuf_id_t      const subbuf_id)
{
  if (mem == NULL)
    return;

  skc_suballocator_subbuf_free(&runtime->allocator.host.temp.suballocator,subbuf_id);
}

//
//
//

void
skc_allocator_host_create(struct skc_runtime * const runtime)
{
  skc_suballocator_create(runtime,
                          &runtime->allocator.host.temp.suballocator,
                          "HOST  ",
                          runtime->config->suballocator.host.subbufs,
                          SKC_RUNTIME_HOST_CACHELINE_SIZE,
                          runtime->config->suballocator.host.size);

  runtime->allocator.host.temp.extent =
    skc_runtime_host_perm_alloc(runtime,
                                SKC_MEM_FLAGS_READ_WRITE,
                                runtime->config->suballocator.host.size);
}

void
skc_allocator_host_dispose(struct skc_runtime * const runtime)
{
  skc_suballocator_dispose(runtime,&runtime->allocator.host.temp.suballocator);

  skc_runtime_host_perm_free(runtime,runtime->allocator.host.temp.extent);
}

//
//
//
