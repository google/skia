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

#include <stdlib.h>

#include "common/cl/assert_cl.h"
#include "extent_cl_12.h"
#include "runtime_cl_12.h"

//
// DURABLE R/W HOST EXTENT -- STANDARD CACHED MEMORY
//

void
skc_extent_phrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_phrw * const extent,
                      size_t                   const size)
{
  extent->hrw = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,size);
}

void
skc_extent_phrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_phrw * const extent)
{
  skc_runtime_host_perm_free(runtime,extent->hrw);
}

//
// DURABLE R/W DEVICE EXTENT -- ALLOCATED FROM DEVICE HEAP
//

void
skc_extent_pdrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_pdrw * const extent,
                      size_t                   const size)
{
  extent->drw = skc_runtime_device_perm_alloc(runtime,
                                              CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                                              size);
}

void
skc_extent_pdrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_pdrw * const extent)
{
  skc_runtime_device_perm_free(runtime,extent->drw);
}

//
// EPHEMERAL DEVICE R/W EXTENT -- ALLOCATED QUICKLY FROM A MANAGED RING
//

void
skc_extent_tdrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_tdrw * const extent,
                      size_t                   const size)
{
  extent->size = size;
  extent->drw  = skc_runtime_device_temp_alloc(runtime,
                                               CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                                               size,&extent->id,NULL);
}

void
skc_extent_tdrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_tdrw * const extent)
{
  skc_runtime_device_temp_free(runtime,extent->drw,extent->id);
}

void
skc_extent_tdrw_zero(struct skc_extent_tdrw * const extent,
                     cl_command_queue         const cq,
                     cl_event               * const event)
{
  if (extent->size == 0)
    return;

  skc_uint const zero = 0;

  cl(EnqueueFillBuffer(cq,
                       extent->drw,
                       &zero,
                       sizeof(zero),
                       0,
                       extent->size,
                       0,NULL,event));
}

//
// DURABLE SMALL EXTENTS BACKING ATOMICS
//

void
skc_extent_phr_pdrw_alloc(struct skc_runtime         * const runtime,
                          struct skc_extent_phr_pdrw * const extent,
                          size_t                       const size)
{
  extent->size = size;
  extent->hr   = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_ONLY,size);
  extent->drw  = skc_runtime_device_perm_alloc(runtime,CL_MEM_READ_WRITE,size);
}

void
skc_extent_phr_pdrw_free(struct skc_runtime         * const runtime,
                         struct skc_extent_phr_pdrw * const extent)
{
  skc_runtime_host_perm_free(runtime,extent->hr);
  skc_runtime_device_perm_free(runtime,extent->drw);
}

void
skc_extent_phr_pdrw_read(struct skc_extent_phr_pdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event)
{
  if (extent->size == 0)
    return;

  cl(EnqueueReadBuffer(cq,
                       extent->drw,
                       CL_FALSE,
                       0,
                       extent->size,
                       extent->hr,
                       0,NULL,event));
}

void
skc_extent_phr_pdrw_zero(struct skc_extent_phr_pdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event)
{
  if (extent->size == 0)
    return;

  skc_uint const zero = 0;

  cl(EnqueueFillBuffer(cq,
                       extent->drw,
                       &zero,
                       sizeof(zero),
                       0,
                       extent->size,
                       0,NULL,event));
}

//
// EPHEMERAL SMALL EXTENTS BACKING ATOMICS
//

void
skc_extent_thr_tdrw_alloc(struct skc_runtime         * const runtime,
                          struct skc_extent_thr_tdrw * const extent,
                          size_t                       const size)
{
  extent->size = size;
  extent->hr   = skc_runtime_host_temp_alloc(runtime,
                                             SKC_MEM_FLAGS_READ_ONLY,
                                             size,&extent->id.hr,NULL);
  extent->drw  = skc_runtime_device_temp_alloc(runtime,
                                               CL_MEM_READ_WRITE,
                                               size,
                                               &extent->id.drw,
                                               NULL);
}

void
skc_extent_thr_tdrw_free(struct skc_runtime         * const runtime,
                         struct skc_extent_thr_tdrw * const extent)
{
  skc_runtime_host_temp_free(runtime,extent->hr,extent->id.hr);
  skc_runtime_device_temp_free(runtime,extent->drw,extent->id.drw);
}

void
skc_extent_thr_tdrw_read(struct skc_extent_thr_tdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event)
{
  if (extent->size == 0)
    return;

  cl(EnqueueReadBuffer(cq,
                       extent->drw,
                       CL_FALSE,
                       0,
                       extent->size,
                       extent->hr,
                       0,NULL,event));
}

void
skc_extent_thr_tdrw_zero(struct skc_extent_thr_tdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event)
{
  if (extent->size == 0)
    return;

  skc_uint const zero = 0;

  cl(EnqueueFillBuffer(cq,
                       extent->drw,
                       &zero,
                       sizeof(zero),
                       0,
                       extent->size,
                       0,NULL,event));
}

//
// DURABLE W/1 HOST RING WITH AN EPHEMERAL R/N DEVICE SNAPSHOT
//

void
skc_extent_phw1g_tdrNs_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phw1g_tdrNs * const extent,
                             size_t                          const size)
{
  extent->hw1 = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_WRITE_ONLY,size);
}

void
skc_extent_phw1g_tdrNs_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phw1g_tdrNs * const extent)
{
  skc_runtime_host_perm_free(runtime,extent->hw1);
}

void
skc_extent_phw1g_tdrNs_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phw1g_tdrNs_snap * const snap)
{
  snap->snap = skc_extent_ring_snap_alloc(runtime,ring);
}

void
skc_extent_phw1g_tdrNs_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phw1g_tdrNs      * const extent,
                                  struct skc_extent_phw1g_tdrNs_snap * const snap,
                                  cl_command_queue                     const cq,
                                  cl_event                           * const event)
{
  struct skc_extent_ring const * const ring = snap->snap->ring;

  skc_uint const count = skc_extent_ring_snap_count(snap->snap);
  size_t   const size  = count * ring->size.elem;

  snap->drN = skc_runtime_device_temp_alloc(runtime,
                                            CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                                            size,&snap->id,NULL);

  if (count == 0)
    return;

  // possibly two copies
  skc_uint const index_lo  = snap->snap->reads & ring->size.mask;
  skc_uint const count_max = ring->size.pow2 - index_lo;
  skc_uint const count_lo  = min(count_max,count);
  size_t   const bytes_lo  = count_lo * ring->size.elem;

  if (count > count_max)
    {
      skc_uint const bytes_hi = (count - count_max) * ring->size.elem;

      cl(EnqueueWriteBuffer(cq,
                            snap->drN,
                            CL_FALSE,
                            bytes_lo,
                            bytes_hi,
                            extent->hw1, // offset_hi = 0
                            0,NULL,NULL));
    }

  size_t const offset_lo = index_lo * ring->size.elem;

  cl(EnqueueWriteBuffer(cq,
                        snap->drN,
                        CL_FALSE,
                        0,
                        bytes_lo,
                        (skc_uchar*)extent->hw1 + offset_lo,
                        0,NULL,event));

}

void
skc_extent_phw1g_tdrNs_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phw1g_tdrNs_snap * const snap)
{
  skc_runtime_device_temp_free(runtime,snap->drN,snap->id);
  skc_extent_ring_snap_free(runtime,snap->snap);
}

//
// DURABLE R/W HOST RING WITH AN EPHEMERAL R/N DEVICE SNAPSHOT
//

void
skc_extent_phrwg_tdrNs_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phrwg_tdrNs * const extent,
                             size_t                          const size)
{
  extent->hrw = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,size); // WRITE-ONCE
}

void
skc_extent_phrwg_tdrNs_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phrwg_tdrNs * const extent)
{
  skc_runtime_host_perm_free(runtime,extent->hrw);
}

void
skc_extent_phrwg_tdrNs_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phrwg_tdrNs_snap * const snap)
{
  snap->snap = skc_extent_ring_snap_alloc(runtime,ring);
}

void
skc_extent_phrwg_tdrNs_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phrwg_tdrNs      * const extent,
                                  struct skc_extent_phrwg_tdrNs_snap * const snap,
                                  cl_command_queue                     const cq,
                                  cl_event                           * const event)
{
  struct skc_extent_ring const * const ring = snap->snap->ring;

  skc_uint const count = skc_extent_ring_snap_count(snap->snap);
  size_t   const size  = count * ring->size.elem;

  snap->drN = skc_runtime_device_temp_alloc(runtime,
                                            CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                                            size,&snap->id,NULL);

  if (count == 0)
    return;

  // possibly two copies
  skc_uint const index_lo  = snap->snap->reads & ring->size.mask;
  skc_uint const count_max = ring->size.pow2 - index_lo;
  skc_uint const count_lo  = min(count_max,count);
  size_t   const bytes_lo  = count_lo * ring->size.elem;

  if (count > count_max)
    {
      skc_uint const count_hi = count - count_max;
      skc_uint const bytes_hi = count_hi * ring->size.elem;

      cl(EnqueueWriteBuffer(cq,
                            snap->drN,
                            CL_FALSE,
                            bytes_lo,
                            bytes_hi,
                            extent->hrw, // offset_hi = 0
                            0,NULL,NULL));
    }

  size_t offset_lo = index_lo * ring->size.elem;

  cl(EnqueueWriteBuffer(cq,
                        snap->drN,
                        CL_FALSE,
                        0,
                        bytes_lo,
                        (skc_uchar*)extent->hrw + offset_lo,
                        0,NULL,event));

}

void
skc_extent_phrwg_tdrNs_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phrwg_tdrNs_snap * const snap)
{
  skc_runtime_device_temp_free(runtime,snap->drN,snap->id);
  skc_extent_ring_snap_free(runtime,snap->snap);
}

//
// DURABLE HOST R/W RING WITH AN EPHEMERAL HOST R/1 SNAPSHOT
//
// Note that because the ring and snapshot are both in host memory and
// the snapshot blocks progress until freed we can simply point the
// fake ephemeral snapshot at the ring's durable extent.
//

void
skc_extent_phrwg_thr1s_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phrwg_thr1s * const extent,
                             size_t                          const size)
{
  extent->hrw = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,size); // WRITE-ONCE
}

void
skc_extent_phrwg_thr1s_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phrwg_thr1s * const extent)
{
  skc_runtime_host_perm_free(runtime,extent->hrw);
}

void
skc_extent_phrwg_thr1s_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phrwg_thr1s_snap * const snap)
{
  snap->snap = skc_extent_ring_snap_alloc(runtime,ring);
}

void
skc_extent_phrwg_thr1s_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phrwg_thr1s      * const extent,
                                  struct skc_extent_phrwg_thr1s_snap * const snap)
{
  struct skc_extent_ring const * const ring = snap->snap->ring;

  skc_uint const count     = skc_extent_ring_snap_count(snap->snap);
  skc_uint const index_lo  = snap->snap->reads & ring->size.mask;
  skc_uint const count_max = ring->size.pow2 - index_lo;

  snap->count.lo = min(count_max,count);
  snap->hr1.lo   = (skc_uchar*)extent->hrw + (index_lo * ring->size.elem);

  if (count > count_max)
    {
      snap->count.hi = count - count_max;
      snap->hr1.hi   = extent->hrw;
    }
  else
    {
      snap->count.hi = 0;
      snap->hr1.hi   = NULL;
    }
}

void
skc_extent_phrwg_thr1s_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phrwg_thr1s_snap * const snap)
{
  skc_extent_ring_snap_free(runtime,snap->snap);
}

//
//
//
