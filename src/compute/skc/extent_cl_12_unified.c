/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// NOTE THAT NONE OF THESE EXTENTS CHECK FOR ZERO-SIZED ALLOCATIONS.
// THAT'S OK FOR NOW.
//

#include <stdlib.h>

#include "runtime_cl_12.h"
#include "extent_cl_12.h"
#include "common/cl/assert_cl.h"

//
// EPHEMERAL MAPPING
//
// ENTIRE EXTENT   MAPPED TO R/W   HOST MEMORY
// ENTIRE EXTENT UNMAPPED TO R/W DEVICE MEMORY
//
// Note: integrated vs. discrete GPUs will have different
// implementations because we don't want a GPU kernel repeatedly
// accessing pinned memory.
//

#if 0

#pragma message("struct skc_extent_thrw_tdrw will be removed once the sorter is installed.")

void
skc_extent_thrw_tdrw_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_thrw_tdrw * const extent,
                           size_t                        const size)
{
  extent->drw = skc_runtime_device_temp_alloc(runtime,
                                              CL_MEM_READ_WRITE /* | CL_MEM_ALLOC_HOST_PTR */,
                                              size,&extent->id,&extent->size);
}

void
skc_extent_thrw_tdrw_free(struct skc_runtime          * const runtime,
                          struct skc_extent_thrw_tdrw * const extent)
{
  skc_runtime_device_temp_free(runtime,extent->drw,extent->id);
}

void *
skc_extent_thrw_tdrw_map_size(struct skc_extent_thrw_tdrw * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event)
{
  cl_int cl_err;

  void * hrw = clEnqueueMapBuffer(cq,extent->drw,
                                  CL_FALSE,
                                  CL_MAP_READ | CL_MAP_WRITE,0,size,
                                  0,NULL,event,&cl_err); cl_ok(cl_err);

  return hrw;
}

void *
skc_extent_thrw_tdrw_map(struct skc_extent_thrw_tdrw * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event)
{
  return skc_extent_thrw_tdrw_map_size(extent,extent->size,cq,event);
}

void
skc_extent_thrw_tdrw_unmap(struct skc_extent_thrw_tdrw * const extent,
                           void                        * const hrw,
                           cl_command_queue              const cq,
                           cl_event                    * const event)
{
  cl(EnqueueUnmapMemObject(cq,extent->drw,hrw,0,NULL,event));
}

#endif

//
// DURABLE MAPPING
//
// ENTIRE EXTENT   MAPPED TO R/W   HOST MEMORY
// ENTIRE EXTENT UNMAPPED TO R/W DEVICE MEMORY
//
// Note: integrated vs. discrete GPUs will have different
// implementations because we don't want a GPU kernel repeatedly
// accessing pinned memory.
//

void
skc_extent_phrw_pdrw_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phrw_pdrw * const extent,
                           size_t                        const size)
{
  cl_int cl_err;

  extent->size = size;
  extent->drw  = clCreateBuffer(runtime->cl.context,
                                CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                size,NULL,&cl_err); cl_ok(cl_err);
}

void
skc_extent_phrw_pdrw_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phrw_pdrw * const extent)
{
  cl(ReleaseMemObject(extent->drw));
}

void *
skc_extent_phrw_pdrw_map_size(struct skc_extent_phrw_pdrw * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event)
{
  cl_int cl_err;

  void * hrw = clEnqueueMapBuffer(cq,extent->drw,
                                  CL_FALSE,
                                  CL_MAP_READ | CL_MAP_WRITE,0,size,
                                  0,NULL,event,&cl_err); cl_ok(cl_err);

  return hrw;
}

void *
skc_extent_phrw_pdrw_map(struct skc_extent_phrw_pdrw * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event)
{
  return skc_extent_phrw_pdrw_map_size(extent,extent->size,cq,event);
}

void
skc_extent_phrw_pdrw_unmap(struct skc_extent_phrw_pdrw * const extent,
                           void                        * const hrw,
                           cl_command_queue              const cq,
                           cl_event                    * const event)
{
  cl(EnqueueUnmapMemObject(cq,extent->drw,hrw,0,NULL,event));
}

//
// DURABLE MAPPING
//
// ENTIRE EXTENT   MAPPED TO R/O   HOST MEMORY
// ENTIRE EXTENT UNMAPPED TO W/O DEVICE MEMORY
//
// Note: integrated vs. discrete GPUs will have different
// implementations because we don't want a GPU kernel repeatedly
// accessing pinned memory.
//

void
skc_extent_phrN_pdwN_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phrN_pdwN * const extent,
                           size_t                        const size)
{
  cl_int cl_err;

  extent->size = size;
  extent->dwN  = clCreateBuffer(runtime->cl.context,
                                CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                size,NULL,&cl_err); cl_ok(cl_err);
}

void
skc_extent_phrN_pdwN_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phrN_pdwN * const extent)
{
  cl(ReleaseMemObject(extent->dwN));
}

void *
skc_extent_phrN_pdwN_map_size(struct skc_extent_phrN_pdwN * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event)
{
  cl_int cl_err;

  void * hrN = clEnqueueMapBuffer(cq,extent->dwN,
                                  CL_FALSE,
                                  CL_MAP_READ,0,size,
                                  0,NULL,event,&cl_err); cl_ok(cl_err);

  return hrN;
}

void *
skc_extent_phrN_pdwN_map(struct skc_extent_phrN_pdwN * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event)
{
  return skc_extent_phrN_pdwN_map_size(extent,extent->size,cq,event);
}

void
skc_extent_phrN_pdwN_unmap(struct skc_extent_phrN_pdwN * const extent,
                           void                        * const hrN,
                           cl_command_queue              const cq,
                           cl_event                    * const event)
{
  cl(EnqueueUnmapMemObject(cq,extent->dwN,hrN,0,NULL,event));
}

//
// DURABLE MAPPING
//
// ENTIRE EXTENT   MAPPED TO W/O   HOST MEMORY
// ENTIRE EXTENT UNMAPPED TO R/O DEVICE MEMORY
//
// Note: integrated vs. discrete GPUs will have different
// implementations because we don't want a GPU kernel repeatedly
// accessing pinned memory.
//

void
skc_extent_phwN_pdrN_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phwN_pdrN * const extent,
                           size_t                        const size)
{
  cl_int cl_err;

  extent->size = size;
  extent->drN  = clCreateBuffer(runtime->cl.context,
                                CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                size,NULL,&cl_err); cl_ok(cl_err);
}

void
skc_extent_phwN_pdrN_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phwN_pdrN * const extent)
{
  cl(ReleaseMemObject(extent->drN));
}

void *
skc_extent_phwN_pdrN_map_size(struct skc_extent_phwN_pdrN * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event)
{
  cl_int cl_err;

  void * hwN = clEnqueueMapBuffer(cq,extent->drN,
                                  CL_FALSE,
                                  CL_MAP_WRITE,0,size,
                                  0,NULL,event,&cl_err); cl_ok(cl_err);

  return hwN;
}

void *
skc_extent_phwN_pdrN_map(struct skc_extent_phwN_pdrN * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event)
{
  return skc_extent_phwN_pdrN_map_size(extent,extent->size,cq,event);
}

void
skc_extent_phwN_pdrN_unmap(struct skc_extent_phwN_pdrN * const extent,
                           void                        * const hwN,
                           cl_command_queue              const cq,
                           cl_event                    * const event)
{
  cl(EnqueueUnmapMemObject(cq,extent->drN,hwN,0,NULL,event));
}

//
//
//
