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

#include "runtime_cl_12.h"
#include "config_cl.h"
#include "common/cl/assert_cl.h"

//
// PERM
//

cl_mem
skc_runtime_device_perm_alloc(struct skc_runtime * const runtime,
                              cl_mem_flags         const flags,
                              size_t               const size)
{
  cl_int cl_err;

  cl_mem mem = clCreateBuffer(runtime->cl.context,
                              flags,
                              size,
                              NULL,
                              &cl_err); cl_ok(cl_err);
  return mem;
}

void
skc_runtime_device_perm_free(struct skc_runtime * const runtime,
                             cl_mem               const mem)
{
  cl(ReleaseMemObject(mem));
}

//
// TEMP
//

cl_mem
skc_runtime_device_temp_alloc(struct skc_runtime * const runtime,
                              cl_mem_flags         const flags,
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

  cl_buffer_region br;

  br.origin = skc_suballocator_subbuf_alloc(&runtime->allocator.device.temp.suballocator,
                                            runtime->scheduler,
                                            size,subbuf_id,&br.size);

  if (subbuf_size != NULL)
    *subbuf_size = br.size;

  cl_int cl_err;

  cl_mem mem = clCreateSubBuffer(runtime->allocator.device.temp.extent,
                                 flags,
                                 CL_BUFFER_CREATE_TYPE_REGION,
                                 &br,
                                 &cl_err); cl_ok(cl_err);

  return mem;
}


void
skc_runtime_device_temp_free(struct skc_runtime * const runtime,
                             cl_mem               const mem,
                             skc_subbuf_id_t      const subbuf_id)
{
  if (mem == NULL)
    return;

  skc_suballocator_subbuf_free(&runtime->allocator.device.temp.suballocator,subbuf_id);

  cl(ReleaseMemObject(mem));
}

//
//
//

void
skc_allocator_device_create(struct skc_runtime * const runtime)
{
  skc_suballocator_create(runtime,
                          &runtime->allocator.device.temp.suballocator,
                          "DEVICE",
                          runtime->config->suballocator.device.subbufs,
                          runtime->cl.align_bytes,
                          runtime->config->suballocator.device.size);

#ifndef NDEBUG
#pragma message("Get rid of CL_MEM_ALLOC_HOST_PTR as soon as the sorter is installed")
  cl_mem_flags const flags = CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR;
#else
  cl_mem_flags const flags = CL_MEM_READ_WRITE;
#endif

  runtime->allocator.device.temp.extent =
    skc_runtime_device_perm_alloc(runtime,
                                  flags,
                                  runtime->config->suballocator.device.size);
}

void
skc_allocator_device_dispose(struct skc_runtime * const runtime)
{
  skc_suballocator_dispose(runtime,&runtime->allocator.device.temp.suballocator);

  skc_runtime_device_perm_free(runtime,runtime->allocator.device.temp.extent);
}

//
//
//
