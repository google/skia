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

#include "types.h"
#include "runtime.h"

//
// The "ring" is a specialized extent designed to accumulate complete
// sequences of commands that are constructed by the host and executed
// on the device.
//
// Note that a sequence of commands is considered to be "complete"
// once a checkpoint has been invoked.
//
// Construction of paths and rasters depends on the checkpointing
// feature.
//
// Note that the ring no longer attempts to account for outstanding
// refcounts on the ring and its snaps.  Waiting for snaps to complete
// is a responsibility best handled elsewhere and up the stack.
//

struct skc_extent_ring
{
  struct skc_extent_ring_snap * head;
  struct skc_extent_ring_snap * last;

  union {
    skc_uint2                   rw;
    struct {
      skc_uint                  reads;  // number of reads
      skc_uint                  writes; // number of writes
    };
  } outer;

  union {
    skc_uint2                   rw;
    struct {
      skc_uint                  reads;  // number of reads
      skc_uint                  writes; // number of writes
    };
  } inner;

  struct {
    skc_uint                    pow2;   // ring size must be pow2
    skc_uint                    mask;   // modulo is a mask because size is pow2
    skc_uint                    snap;   // max elements in a snapshot (not req'd to be pow2)
    skc_uint                    elem;   // size of element in bytes
  } size;
};

//
//
//

void
skc_extent_ring_init(struct skc_extent_ring * const ring,
                     skc_uint                 const size_pow2,
                     skc_uint                 const size_snap,
                     skc_uint                 const size_elem);

//
//
//

skc_bool
skc_extent_ring_rem(struct skc_extent_ring const * const ring);

skc_bool
skc_extent_ring_is_full(struct skc_extent_ring const * const ring);

skc_uint
skc_extent_ring_wip_count(struct skc_extent_ring const * const ring);

skc_uint
skc_extent_ring_wip_rem(struct skc_extent_ring const * const ring);

skc_bool
skc_extent_ring_wip_is_full(struct skc_extent_ring const * const ring);

skc_uint
skc_extent_ring_wip_index_inc(struct skc_extent_ring * const ring);

//
//
//

void
skc_extent_ring_checkpoint(struct skc_extent_ring * const ring);

//
//
//

struct skc_extent_ring_snap
{
  struct skc_extent_ring      * ring;   // parent ring
  struct skc_extent_ring_snap * next;   // next snap

  skc_uint                      reads;  // number of reads
  skc_uint                      writes; // number of writes

  skc_bool                      is_free;

  skc_subbuf_id_t               id;     // id of host temp suballocation
};

//
// For now, all ring snaps allocations occur in "host temporary"
// memory.
//

struct skc_extent_ring_snap *
skc_extent_ring_snap_alloc(struct skc_runtime     * const runtime,
                           struct skc_extent_ring * const ring);

void
skc_extent_ring_snap_free(struct skc_runtime          * const runtime,
                          struct skc_extent_ring_snap * const snap);

//
//
//

skc_uint
skc_extent_ring_snap_count(struct skc_extent_ring_snap const * const snap);

skc_uint
skc_extent_ring_snap_from(struct skc_extent_ring_snap const * const snap);

skc_uint
skc_extent_ring_snap_to(struct skc_extent_ring_snap const * const snap);

//
//
//

