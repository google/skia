/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"
#include "runtime.h"
#include "cq_pool_cl.h"
#include "handle_pool_cl_12.h"
#include "block_pool_cl_12.h"
#include "allocator_device_cl.h"

//
// FIXME -- two parts:
//
// 1. directly access the structures in the runtime sub-struct implementations
// 2. possibly wall off the non-platform-specific structs into a sub structure
//

struct skc_runtime
{
  //
  // state visible to device
  //
  struct {
    cl_context                     context;
    cl_device_id                   device_id;
    cl_uint                        align_bytes;
  } cl;

  struct {
    struct skc_allocator_host      host;
    struct skc_allocator_device    device;
  } allocator;

  struct skc_cq_pool               cq_pool;

  struct skc_block_pool            block_pool;

  struct skc_handle_pool           handle_pool;

  //
  // state that is slightly opaque (for now)
  //
  struct skc_scheduler           * scheduler;

  struct skc_grid_deps           * deps;

  struct skc_config const        * config; // FIXME: config will be determined by device with some opportunities to resize

  struct skc_device              * device; // opaque bundle of kernels

  struct hs_cl      const        * hs;     // opaque hotsort
};

//
// Creation and disposal intitializes context and may rely on other
// context resources like the scheduler
//

skc_err
skc_runtime_cl_12_create(struct skc_context * const context,
                         cl_context                 context_cl,
                         cl_device_id               device_id_cl);

skc_err
skc_runtime_cl_12_dispose(struct skc_context * const context);

//
// HOST HANDLE RETAIN/RELEASE/FLUSH
//

skc_err
skc_runtime_path_host_retain(struct skc_runtime * const runtime,
                             skc_path_t   const *       paths,
                             uint32_t                   count);

skc_err
skc_runtime_raster_host_retain(struct skc_runtime * const runtime,
                               skc_raster_t const *       rasters,
                               uint32_t                   count);


skc_err
skc_runtime_path_host_release(struct skc_runtime * const runtime,
                              skc_path_t   const *       paths,
                              uint32_t                   count);

skc_err
skc_runtime_raster_host_release(struct skc_runtime * const runtime,
                                skc_raster_t const *       rasters,
                                uint32_t                   count);


skc_err
skc_runtime_path_host_flush(struct skc_runtime * const runtime,
                            skc_path_t   const *       paths,
                            uint32_t                   count);

skc_err
skc_runtime_raster_host_flush(struct skc_runtime * const runtime,
                              skc_raster_t const *       rasters,
                              uint32_t                   count);

//
// DEVICE/PIPELINE HANDLE ACQUIRE/RETAIN/RELEASE
//
// The retain operations pre-validate handles
//

skc_handle_t
skc_runtime_handle_device_acquire(struct skc_runtime * const runtime);

skc_err
skc_runtime_handle_device_validate_retain(struct skc_runtime       * const runtime,
                                          skc_typed_handle_type_e    const handle_type,
                                          skc_typed_handle_t const *       typed_handles,
                                          uint32_t                         count);

void
skc_runtime_handle_device_retain(struct skc_runtime * const runtime,
                                 skc_handle_t const *       handles,
                                 uint32_t                   count);

void
skc_runtime_path_device_release(struct skc_runtime * const runtime,
                                skc_handle_t const *       handles,
                                uint32_t                   count);

void
skc_runtime_raster_device_release(struct skc_runtime * const runtime,
                                  skc_handle_t const *       handles,
                                  uint32_t                   count);

//
// We only use in-order command queues in the pipeline
//

cl_command_queue
skc_runtime_acquire_cq_in_order(struct skc_runtime * const runtime);

void
skc_runtime_release_cq_in_order(struct skc_runtime * const runtime,
                                cl_command_queue           cq);

//
// DEVICE MEMORY ALLOCATION
//

cl_mem
skc_runtime_device_perm_alloc(struct skc_runtime * const runtime,
                              cl_mem_flags         const flags,
                              size_t               const size);

void
skc_runtime_device_perm_free(struct skc_runtime * const runtime,
                             cl_mem               const mem);

cl_mem
skc_runtime_device_temp_alloc(struct skc_runtime * const runtime,
                              cl_mem_flags         const flags,
                              size_t               const size,
                              skc_subbuf_id_t    * const subbuf_id,
                              size_t             * const subbuf_size);

void
skc_runtime_device_temp_free(struct skc_runtime * const runtime,
                             cl_mem               const mem,
                             skc_subbuf_id_t      const subbuf_id);

//
//
//

void
skc_runtime_cl_12_debug(struct skc_context * const context);

//
//
//
