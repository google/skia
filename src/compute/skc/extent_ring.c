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

#include <stdbool.h>

#include "extent_ring.h"
#include "macros.h"

//
//
//

void
skc_extent_ring_init(struct skc_extent_ring * const ring,
                     skc_uint                 const size_pow2,
                     skc_uint                 const size_snap,
                     skc_uint                 const size_elem)
{
  ring->head      = NULL;
  ring->last      = NULL;

  ring->outer.rw  = (skc_uint2){ 0 };
  ring->inner.rw  = (skc_uint2){ 0 };

  // FIXME -- assert size is pow2 -- either here or statically in the config

  ring->size.pow2 = size_pow2;
  ring->size.mask = size_pow2 - 1;
  ring->size.snap = size_snap;
  ring->size.elem = size_elem;
}

//
//
//

skc_uint
skc_extent_ring_rem(struct skc_extent_ring const * const ring)
{
  return ring->size.pow2 - (ring->outer.writes - ring->outer.reads);
}

skc_bool
skc_extent_ring_is_full(struct skc_extent_ring const * const ring)
{
  return (ring->outer.writes - ring->outer.reads) == ring->size.pow2;
}

skc_uint
skc_extent_ring_wip_count(struct skc_extent_ring const * const ring)
{
  return ring->outer.writes - ring->inner.reads;
}

skc_uint
skc_extent_ring_wip_rem(struct skc_extent_ring const * const ring)
{
  return SKC_MIN_MACRO(skc_extent_ring_rem(ring),ring->size.snap) - skc_extent_ring_wip_count(ring);
}

skc_bool
skc_extent_ring_wip_is_full(struct skc_extent_ring const * const ring)
{
  return skc_extent_ring_wip_count(ring) == SKC_MIN_MACRO(skc_extent_ring_rem(ring),ring->size.snap);
}

skc_uint
skc_extent_ring_wip_index_inc(struct skc_extent_ring * const ring)
{
  return ring->outer.writes++ & ring->size.mask;
}

//
//
//

void
skc_extent_ring_checkpoint(struct skc_extent_ring * const ring)
{
  ring->inner.writes = ring->outer.writes;
}

//
//
//

struct skc_extent_ring_snap *
skc_extent_ring_snap_alloc(struct skc_runtime     * const runtime,
                           struct skc_extent_ring * const ring)
{
  skc_subbuf_id_t id;

  struct skc_extent_ring_snap * snap =
    skc_runtime_host_temp_alloc(runtime,
                                SKC_MEM_FLAGS_READ_WRITE,
                                sizeof(*snap),&id,NULL);
  // save the id
  snap->id      = id;

  // back point to parent
  snap->ring    = ring;
  snap->next    = NULL;

  // save the inner boundaries of the ring to the snapshot
  snap->reads   = ring->inner.reads;
  snap->writes  = ring->inner.reads = ring->inner.writes;

  // mark not free
  snap->is_free = false;

  // attach snap to ring
  if (ring->head == NULL)
    {
      ring->head = snap;
      ring->last = snap;
    }
  else
    {
      ring->last->next = snap;
      ring->last       = snap;
    }

  return snap;
}

//
//
//

void
skc_extent_ring_snap_free(struct skc_runtime          * const runtime,
                          struct skc_extent_ring_snap * const snap)
{
  // snap will be lazily freed
  snap->is_free = true;

  //
  // if this snapshot is no longer referenced then try to dispose of
  // the ring buffer's leading unreferenced snapshots
  //
  struct skc_extent_ring      * const ring = snap->ring;
  struct skc_extent_ring_snap *       curr = ring->head;

  if (!curr->is_free)
    return;

  do {
    // increment read counter
    ring->outer.reads = curr->writes;

    struct skc_extent_ring_snap * const next = curr->next;

    skc_runtime_host_temp_free(runtime,curr,curr->id);

    curr = next;

    // this was the last snap...
    if (curr == NULL)
      {
        ring->last = NULL;
        break;
      }

    // is the next free?
  } while (curr->is_free);

  // update head
  ring->head = curr;
}

//
//
//

skc_uint
skc_extent_ring_snap_count(struct skc_extent_ring_snap const * const snap)
{
  return snap->writes - snap->reads;
}

skc_uint
skc_extent_ring_snap_from(struct skc_extent_ring_snap const * const snap)
{
  return snap->reads & snap->ring->size.mask;
}

skc_uint
skc_extent_ring_snap_to(struct skc_extent_ring_snap const * const snap)
{
  return snap->writes & snap->ring->size.mask;
}

//
//
//
