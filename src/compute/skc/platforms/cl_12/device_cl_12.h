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

#include <CL/opencl.h>

//
//
//

#define SKC_CL_ARG(arg) sizeof(arg),&arg

//
//
//

typedef enum skc_device_kernel_id {
  SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_IDS,
  SKC_DEVICE_KERNEL_ID_BLOCK_POOL_INIT_ATOMICS,

  SKC_DEVICE_KERNEL_ID_PATHS_ALLOC,
  SKC_DEVICE_KERNEL_ID_PATHS_COPY,

  SKC_DEVICE_KERNEL_ID_FILLS_EXPAND,

  SKC_DEVICE_KERNEL_ID_RASTERIZE_ALL,
  SKC_DEVICE_KERNEL_ID_RASTERIZE_LINES,
  SKC_DEVICE_KERNEL_ID_RASTERIZE_QUADS,
  SKC_DEVICE_KERNEL_ID_RASTERIZE_CUBICS,
  SKC_DEVICE_KERNEL_ID_RASTERIZE_RAT_QUADS,
  SKC_DEVICE_KERNEL_ID_RASTERIZE_RAT_CUBICS,

  SKC_DEVICE_KERNEL_ID_SEGMENT_TTRK,
  SKC_DEVICE_KERNEL_ID_RASTERS_ALLOC,

  SKC_DEVICE_KERNEL_ID_PREFIX,
  SKC_DEVICE_KERNEL_ID_PLACE,
  SKC_DEVICE_KERNEL_ID_SEGMENT_TTCK,

  SKC_DEVICE_KERNEL_ID_RENDER,

  SKC_DEVICE_KERNEL_ID_PATHS_RECLAIM,
  SKC_DEVICE_KERNEL_ID_RASTERS_RECLAIM,

  //
  SKC_DEVICE_KERNEL_ID_COUNT

} skc_device_kernel_id;

//
//
//

void
skc_device_create(struct skc_runtime * const runtime);


void
skc_device_dispose(struct skc_runtime * const runtime);


//
// multi-threading/context/device requires multiple kernel instances
//

cl_kernel
skc_device_acquire_kernel(struct skc_device  * const device,
                          skc_device_kernel_id const type);

void
skc_device_release_kernel(struct skc_device  * const device,
                          cl_kernel                  kernel);

//
// grid shape can vary greatly by target platform
//
void
skc_device_enqueue_kernel(struct skc_device  * const device,
                          skc_device_kernel_id const type,
                          cl_command_queue           cq,
                          cl_kernel                  kernel,
                          size_t               const work_size,
                          cl_uint                    num_events_in_wait_list,
                          cl_event const     * const event_wait_list,
                          cl_event           * const event);

//
//
//
