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

#include "macros.h"
#include "handle.h"
#include "extent_cl_12.h"
#include "device_cl_12.h"

//
// FIXME -- THIS DOCUMENTATION IS STALE NOW THAT A REFERENCE COUNT REP
// IS A {HOST:DEVICE} PAIR.
//
// Host-side handle pool
//
// The bulk size of the three extents is currently 6 bytes of overhead
// per number of host handles.  The number of host handles is usually
// less than the number of blocks in the pool.  Note that the maximum
// number of blocks is 2^27.
//
// A practical instantiation might provide a combined 2^20 path and
// raster host handles. This would occupy 6 MB of host RAM for the
// 32-bit handle, 8-bit reference count and 8-bit handle-to-grid map.
//
// Also note that we could use isolated/separate path and raster block
// pools. Worst case, this would double the memory footprint of SKC.
//
// Host-side handle reference count
//
//   [0      ] : release
//   [1..UMAX] : retain
//
// In a garbage-collected environment we might want to rely on an
// existing mechanism for determing whether a handle is live.
//
// Otherwise, we probably want to have a 16 or 32-bit ref count.
//
// The handle reference count is defensive and will not allow the host
// to underflow a handle that's still retained by the pipeline.
//
// The single reference counter is split into host and device counts.
//

union skc_handle_refcnt
{
  skc_ushort  hd; // host and device

  struct {
    skc_uchar h;  // host
    skc_uchar d;  // device
  };
};

SKC_STATIC_ASSERT(SKC_MEMBER_SIZE(union skc_handle_refcnt,hd) ==
                  SKC_MEMBER_SIZE(union skc_handle_refcnt,h) +
                  SKC_MEMBER_SIZE(union skc_handle_refcnt,d));

//
//
//

struct skc_handle_bih
{
  skc_uint       block;
  skc_uint       rem;
  skc_handle_t * handles;
};

struct skc_handle_reclaim
{
  struct skc_handle_bih bih;

  cl_kernel             kernel;
  skc_device_kernel_id  kernel_id;
};

union skc_handle_reclaim_rec
{
  // ELEMENT  0
  struct skc_runtime * runtime;

  // ELEMENT  1
  struct {
    skc_uint           rem;   // # of available records
    skc_uint           head;  // index of first record
  };

  // ELEMENTS 2+
  struct {
    skc_uint           index; // index of this record -- never modified
    union {
      skc_uint         next;  // index of next record
      skc_uint         block; // block index of reclaimed handles
    };
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_handle_reclaim_rec) == sizeof(skc_uint2));

//
//
//

typedef enum skc_handle_reclaim_type_e {

  SKC_HANDLE_RECLAIM_TYPE_PATH,
  SKC_HANDLE_RECLAIM_TYPE_RASTER,

  SKC_HANDLE_RECLAIM_TYPE_COUNT

} skc_handle_reclaim_type_e;

struct skc_handle_pool
{
  //
  // FIXME -- should we be pedantic and make these always-host-side
  // allocations "extents" as well?  I think it's OK not being an
  // extent structure for now and is mostly consistent with the rest
  // of the code.
  //
  // FIXME -- the cbs[] array is a little idiosyncratic but the intent
  // is to avoid storing the 64-bit backpointer inside of every single
  // record.  This can be harmonized later.  Note that only a few
  // hundred outstanding callbacks would represent many many subgroups
  // of work and would fully occupy the GPU (if we allow it).
  //
  //
  struct skc_extent_pdrw         map;     // device-managed extent mapping a host handle to device block id

  struct {
    skc_handle_t               * indices; // array of individual host handles -- fragmented into blocks
    union skc_handle_refcnt    * refcnts; // array of reference counts indexed by an individual handle
    skc_uint                     count;
  } handle;

  struct {
    skc_uint                   * indices; // stack of indices to fixed-size blocks of host handles
    skc_uint                     count;   // number of handles -- valid from [0,size)
    skc_uint                     width;   // width of a fixed-size block of handles
    skc_uint                     tos;     // grows upward   / push++ / --pop / # fixed-size blocks for reading
    skc_uint                     bos;     // grows downward / --push / pop++ / # fixed-size blocks for writing
  } block;

  union skc_handle_reclaim_rec * recs;    // array of reclaim records

  struct skc_handle_bih          acquire;
  struct skc_handle_reclaim      reclaim[SKC_HANDLE_RECLAIM_TYPE_COUNT];
};

//
//
//

void
skc_handle_pool_create(struct skc_runtime     * const runtime,
                       struct skc_handle_pool * const handle_pool,
                       skc_uint                 const size,
                       skc_uint                 const width,
                       skc_uint                 const recs);

void
skc_handle_pool_dispose(struct skc_runtime     * const runtime,
                        struct skc_handle_pool * const handle_pool);

//
//
//
