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

#include "suballocator.h"
#include "extent_ring.h"

//
// Legend:
//
//   p  :  durable
//   t  :  ephemeral
//   h  :  host
//   d  :  device
//   r  :  read
//   w  :  write
//   1  :  once -- e.g. w1 is 'write-once'
//   N  :  many -- e.g. rN is 'read-many'
//   g  :  ring
//   s  :  ring snapshot
//
// Notes:
//
//   rw :  for now, read-write implies read-write many
//

//
// DURABLE R/W HOST EXTENT -- STANDARD CACHED MEMORY
//

struct skc_extent_phrw
{
  void * hrw;
};

void
skc_extent_phrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_phrw * const extent,
                      size_t                   const size);

void
skc_extent_phrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_phrw * const extent);

//
// DURABLE R/W DEVICE EXTENT -- ALLOCATED FROM DEVICE HEAP
//

struct skc_extent_pdrw
{
  cl_mem drw;
};

void
skc_extent_pdrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_pdrw * const extent,
                      size_t                   const size);

void
skc_extent_pdrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_pdrw * const extent);

//
// EPHEMERAL DEVICE R/W EXTENT -- ALLOCATED QUICKLY FROM A MANAGED RING
//

struct skc_extent_tdrw
{
  size_t          size;
  cl_mem          drw;
  skc_subbuf_id_t id;
};

void
skc_extent_tdrw_alloc(struct skc_runtime     * const runtime,
                      struct skc_extent_tdrw * const extent,
                      size_t                   const size);

void
skc_extent_tdrw_free(struct skc_runtime     * const runtime,
                     struct skc_extent_tdrw * const extent);

void
skc_extent_tdrw_zero(struct skc_extent_tdrw * const extent,
                     cl_command_queue         const cq,
                     cl_event               * const event);

//
// DURABLE SMALL EXTENTS BACKING ATOMICS
//

struct skc_extent_phr_pdrw
{
  size_t size; // must be multiple of words
  void * hr;
  cl_mem drw;
};

void
skc_extent_phr_pdrw_alloc(struct skc_runtime         * const runtime,
                          struct skc_extent_phr_pdrw * const extent,
                          size_t                       const size);

void
skc_extent_phr_pdrw_free(struct skc_runtime         * const runtime,
                         struct skc_extent_phr_pdrw * const extent);

void
skc_extent_phr_pdrw_read(struct skc_extent_phr_pdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event);

void
skc_extent_phr_pdrw_zero(struct skc_extent_phr_pdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event);

//
// EPHEMERAL SMALL EXTENTS BACKING ATOMICS
//

struct skc_extent_thr_tdrw
{
  size_t            size; // must be multiple of words

  void            * hr;
  cl_mem            drw;

  struct {
    skc_subbuf_id_t hr;
    skc_subbuf_id_t drw;
  } id;
};

void
skc_extent_thr_tdrw_alloc(struct skc_runtime         * const runtime,
                          struct skc_extent_thr_tdrw * const extent,
                          size_t                       const size);

void
skc_extent_thr_tdrw_free(struct skc_runtime         * const runtime,
                         struct skc_extent_thr_tdrw * const extent);

void
skc_extent_thr_tdrw_read(struct skc_extent_thr_tdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event);

void
skc_extent_thr_tdrw_zero(struct skc_extent_thr_tdrw * const extent,
                         cl_command_queue             const cq,
                         cl_event                   * const event);

//
// DURABLE W/1 HOST RING WITH AN EPHEMERAL R/N DEVICE SNAPSHOT
//

struct skc_extent_phw1g_tdrNs
{
  void * hw1;
};

struct skc_extent_phw1g_tdrNs_snap
{
  struct skc_extent_ring_snap * snap;
  cl_mem                        drN;
  skc_subbuf_id_t               id;
};

void
skc_extent_phw1g_tdrNs_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phw1g_tdrNs * const extent,
                             size_t                          const size);

void
skc_extent_phw1g_tdrNs_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phw1g_tdrNs * const extent);

void
skc_extent_phw1g_tdrNs_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phw1g_tdrNs_snap * const snap);

void
skc_extent_phw1g_tdrNs_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phw1g_tdrNs      * const extent,
                                  struct skc_extent_phw1g_tdrNs_snap * const snap,
                                  cl_command_queue                     const cq,
                                  cl_event                           * const event);

void
skc_extent_phw1g_tdrNs_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phw1g_tdrNs_snap * const snap);

//
// DURABLE R/W HOST RING WITH AN EPHEMERAL R/N DEVICE SNAPSHOT
//

struct skc_extent_phrwg_tdrNs
{
  void * hrw;
};

struct skc_extent_phrwg_tdrNs_snap
{
  struct skc_extent_ring_snap * snap;
  cl_mem                        drN;
  skc_subbuf_id_t               id;
};

void
skc_extent_phrwg_tdrNs_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phrwg_tdrNs * const extent,
                             size_t                          const size);

void
skc_extent_phrwg_tdrNs_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phrwg_tdrNs * const extent);

void
skc_extent_phrwg_tdrNs_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phrwg_tdrNs_snap * const snap);

void
skc_extent_phrwg_tdrNs_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phrwg_tdrNs      * const extent,
                                  struct skc_extent_phrwg_tdrNs_snap * const snap,
                                  cl_command_queue                     const cq,
                                  cl_event                           * const event);

void
skc_extent_phrwg_tdrNs_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phrwg_tdrNs_snap * const snap);

//
// DURABLE HOST R/W RING WITH AN EPHEMERAL HOST R/1 SNAPSHOT
//
// Note that because the ring and snapshot are both in host memory and
// the snapshot blocks progress until freed we can simply point the
// fake ephemeral snapshot at the ring's durable extent.
//

struct skc_extent_phrwg_thr1s
{
  void * hrw;
};

struct skc_extent_phrwg_thr1s_snap
{
  struct skc_extent_ring_snap * snap;

  struct {
    skc_uint                    lo;
    skc_uint                    hi;
  } count;

  struct {
    void                      * lo;
    void                      * hi;
  } hr1;
};

void
skc_extent_phrwg_thr1s_alloc(struct skc_runtime            * const runtime,
                             struct skc_extent_phrwg_thr1s * const extent,
                             size_t                          const size);

void
skc_extent_phrwg_thr1s_free(struct skc_runtime            * const runtime,
                            struct skc_extent_phrwg_thr1s * const extent);

void
skc_extent_phrwg_thr1s_snap_init(struct skc_runtime                 * const runtime,
                                 struct skc_extent_ring             * const ring,
                                 struct skc_extent_phrwg_thr1s_snap * const snap);

void
skc_extent_phrwg_thr1s_snap_alloc(struct skc_runtime                 * const runtime,
                                  struct skc_extent_phrwg_thr1s      * const extent,
                                  struct skc_extent_phrwg_thr1s_snap * const snap);

void
skc_extent_phrwg_thr1s_snap_free(struct skc_runtime                 * const runtime,
                                 struct skc_extent_phrwg_thr1s_snap * const snap);

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
struct skc_extent_thrw_tdrw
{
  size_t          size;
  cl_mem          drw;
  skc_subbuf_id_t id;
};

void
skc_extent_thrw_tdrw_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_thrw_tdrw * const extent,
                           size_t                        const size);

void
skc_extent_thrw_tdrw_free(struct skc_runtime          * const runtime,
                          struct skc_extent_thrw_tdrw * const extent);

void *
skc_extent_thrw_tdrw_map_size(struct skc_extent_thrw_tdrw * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event);

void *
skc_extent_thrw_tdrw_map(struct skc_extent_thrw_tdrw * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event);

void
skc_extent_thrw_tdrw_unmap(struct skc_extent_thrw_tdrw * const extent,
                           void                        * const hrN,
                           cl_command_queue              const cq,
                           cl_event                    * const event);
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

struct skc_extent_phrw_pdrw
{
  size_t size;
  cl_mem drw;
};

void
skc_extent_phrw_pdrw_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phrw_pdrw * const extent,
                           size_t                        const size);

void
skc_extent_phrw_pdrw_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phrw_pdrw * const extent);

void *
skc_extent_phrw_pdrw_map_size(struct skc_extent_phrw_pdrw * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event);

void *
skc_extent_phrw_pdrw_map(struct skc_extent_phrw_pdrw * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event);

void
skc_extent_phrw_pdrw_unmap(struct skc_extent_phrw_pdrw * const extent,
                           void                        * const hrN,
                           cl_command_queue              const cq,
                           cl_event                    * const event);

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

struct skc_extent_phrN_pdwN
{
  size_t size;
  cl_mem dwN;
};

void
skc_extent_phrN_pdwN_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phrN_pdwN * const extent,
                           size_t                        const size);

void
skc_extent_phrN_pdwN_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phrN_pdwN * const extent);

void *
skc_extent_phrN_pdwN_map_size(struct skc_extent_phrN_pdwN * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event);

void *
skc_extent_phrN_pdwN_map(struct skc_extent_phrN_pdwN * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event);

void
skc_extent_phrN_pdwN_unmap(struct skc_extent_phrN_pdwN * const extent,
                           void                        * const hrN,
                           cl_command_queue              const cq,
                           cl_event                    * const event);

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

struct skc_extent_phwN_pdrN
{
  size_t size;
  cl_mem drN;
};

void
skc_extent_phwN_pdrN_alloc(struct skc_runtime          * const runtime,
                           struct skc_extent_phwN_pdrN * const extent,
                           size_t                        const size);

void
skc_extent_phwN_pdrN_free(struct skc_runtime          * const runtime,
                          struct skc_extent_phwN_pdrN * const extent);

void *
skc_extent_phwN_pdrN_map_size(struct skc_extent_phwN_pdrN * const extent,
                              size_t                        const size,
                              cl_command_queue              const cq,
                              cl_event                    * const event);

void *
skc_extent_phwN_pdrN_map(struct skc_extent_phwN_pdrN * const extent,
                         cl_command_queue              const cq,
                         cl_event                    * const event);

void
skc_extent_phwN_pdrN_unmap(struct skc_extent_phwN_pdrN * const extent,
                           void                        * const hwm,
                           cl_command_queue              const cq,
                           cl_event                    * const event);

//
//
//
