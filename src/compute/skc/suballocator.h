/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
// This is a suballocator for a large extent typically less than 4 GB.
//
// The SKC pipeline will use this for ephemeral host and device memory
// allocations.  The lifetime of an allocation is typically
// milliseconds or less and is associated with either a single kernel
// or a sub-pipeline.
//
// Because of this, a relatively small number of allocations (10's)
// will be outstanding at any time so the implementation can
// reasonably be very simplistic and optimize for this case.
//
// Also, if either memory or subbuffer nodes aren't available the
// suballocator will block and pump the pipeline's scheduler until it
// can proceed.
//
// Note that this implementation is single-threaded and the
// suballocator's state may have been altered after pumping the
// scheduler.
//

#include "types.h"

//
// It's practical for the subbuf_id to be either 16 bits or maybe even
// 8 bits if the number of outstanding subbufs is in the thousands (16
// bits) or under 256 (8 bits).
//

typedef skc_ushort skc_subbuf_id_t;
typedef skc_uint   skc_subbuf_size_t; // <4GB
// typedef size_t  skc_subbuf_size_t; // >4GB

//
//
//

struct skc_subbuf
{
  struct skc_subbuf * prev;
  struct skc_subbuf * next;

  skc_subbuf_size_t   size;
  skc_subbuf_size_t   origin;

  skc_uint            idx; // ids[] index of subbuf in available state
  skc_uint            inuse;
};

//
//
//

struct skc_suballocator
{
  struct skc_subbuf  * subbufs;

  skc_subbuf_id_t    * ids;   // [<-AVAIL-><-empty-><-SPARE->]

  struct {
    skc_uint           avail;
    skc_uint           spare;
  } rem; // inuse = count - (avail + spare)

  skc_uint             align; // required pow2 alignment
  skc_uint             count; // number of subbufs

  skc_subbuf_size_t    size;  // size of memory extent
  skc_subbuf_size_t    total;

  char const *         name;
};

//
//
//

void
skc_suballocator_create(struct skc_runtime      * const runtime,
                        struct skc_suballocator * const suballocator,
                        char              const * const name,
                        skc_uint                  const subbufs,
                        size_t                    const align,
                        size_t                    const size);

void
skc_suballocator_dispose(struct skc_runtime      * const runtime,
                         struct skc_suballocator * const suballocator);

//
//
//

size_t
skc_suballocator_subbuf_alloc(struct skc_suballocator * const suballocator,
                              struct skc_scheduler    * const scheduler,
                              size_t                    const size,
                              skc_subbuf_id_t         * const subbuf_id,
                              size_t                  * const subbuf_size);

void
skc_suballocator_subbuf_free(struct skc_suballocator * const suballocator,
                             skc_subbuf_id_t                 subbuf_id);

//
//
//
