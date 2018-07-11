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

#include <assert.h>
#include <memory.h>

#include "runtime_cl_12.h"
#include "scheduler.h"

//
//
//

#ifndef NDEBUG

#include <stdio.h>

#define SKC_SUBALLOCATOR_DEBUG_ALLOC(suballocator,subbuf_id,ss)         \
  fprintf(stderr,                                                       \
          "suballocator %s : [ %4u ] : alloc( %9u ) @ %4u = %u\n",      \
          suballocator->name,                                           \
          suballocator->rem.avail,                                      \
          (skc_uint)ss,                                                 \
          subbuf_id,                                                    \
          (skc_uint)suballocator->total);

#define SKC_SUBALLOCATOR_DEBUG_FREE(suballocator,subbuf_id,ss)          \
  fprintf(stderr,                                                       \
          "suballocator %s : [ %4u ] : free ( %9u ) @ %4u = %u\n",      \
          suballocator->name,                                           \
          suballocator->rem.avail,                                      \
          (skc_uint)ss,                                                 \
          subbuf_id,                                                    \
          (skc_uint)suballocator->total);

#else

#define SKC_SUBALLOCATOR_DEBUG_ALLOC(suballocator,subbuf_id,ss)
#define SKC_SUBALLOCATOR_DEBUG_FREE(suballocator,subbuf_id,ss)

#endif

//
//
//

void
skc_suballocator_create(struct skc_runtime      * const runtime,
                        struct skc_suballocator * const suballocator,
                        char              const * const name,
                        skc_uint                  const subbufs,
                        size_t                    const align,
                        size_t                    const size)
{
  size_t const subbufs_size = sizeof(*suballocator->subbufs) * subbufs;

  // allocate array of subbuf records
  suballocator->subbufs = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,subbufs_size);

  // zero subbufs
  memset(suballocator->subbufs,0,subbufs_size);

  // initialize starting subbuf
  suballocator->subbufs[0].size = (skc_subbuf_size_t)size;

  // allocate array of ids
  suballocator->ids = skc_runtime_host_perm_alloc(runtime,
                                                  SKC_MEM_FLAGS_READ_WRITE,
                                                  sizeof(*suballocator->ids) * subbufs);
  for (skc_uint ii=0; ii<subbufs; ii++)
    suballocator->ids[ii] = ii;

  suballocator->rem.avail = 1;
  suballocator->rem.spare = subbufs - 1;

  suballocator->align     = (skc_uint)align;
  suballocator->count     = subbufs;

  suballocator->size      = (skc_subbuf_size_t)size;
  suballocator->total     = 0;

  suballocator->name      = name;
}

void
skc_suballocator_dispose(struct skc_runtime      * const runtime,
                         struct skc_suballocator * const suballocator)
{
  skc_runtime_host_perm_free(runtime,suballocator->ids);
  skc_runtime_host_perm_free(runtime,suballocator->subbufs);
}


//
// Sets id and returns origin
//

size_t
skc_suballocator_subbuf_alloc(struct skc_suballocator * const suballocator,
                              struct skc_scheduler    * const scheduler,
                              size_t                    const size,
                              skc_subbuf_id_t         * const subbuf_id,
                              size_t                  * const subbuf_size)
{
  //
  // Note that we can't deadlock here because everything allocated is
  // expected to be freed within msecs.  Worst case, we wait for a
  // availability of resources while a fully utilized GPU is making
  // forward progress on kernels.
  //
  // This behavior should guide the sizing of the suballocator's
  // number of subbuffers and extent.
  //
  // We want to allocate a large enough extent and enough subbuffer
  // records so that the CPU/GPU is never starved.
  //

  // round up the size
  skc_subbuf_size_t const size_ru = (skc_subbuf_size_t)SKC_ROUND_UP_POW2(size,suballocator->align);

  // save it
  if (subbuf_size != NULL)
    *subbuf_size = size_ru;

  //
  // We precheck to see there is at least one region of memory
  // available but do not check to see if there is a spare. Instead,
  // we simply keep looking for an exact fit.
  //
  skc_subbuf_id_t * const ids = suballocator->ids;

  while (true)
    {
      skc_uint avail_rem = suballocator->rem.avail;
      skc_uint spare_rem = suballocator->rem.spare;

      for (skc_uint avail_idx=0; avail_idx<avail_rem; avail_idx++)
        {
          skc_subbuf_id_t     const avail_id = ids[avail_idx];
          struct skc_subbuf * const avail    = suballocator->subbufs + avail_id;

          assert(avail->inuse == 0);

          if (avail->size == size_ru) // size matches exactly
            {
              suballocator->total += size_ru;

              // return this id
              *subbuf_id = avail_id;

              SKC_SUBALLOCATOR_DEBUG_ALLOC(suballocator,avail_id,size_ru);

              // mark the subbuffer as in use
              avail->inuse += 1;

              assert(avail->inuse == 1);

              // update rem avail count
              suballocator->rem.avail = --avail_rem;

              // replace now inuse id with last avail id
              if ((avail_rem > 0) && (avail_idx != avail_rem))
                {
                  skc_subbuf_id_t     const last_id = ids[avail_rem];
                  struct skc_subbuf * const last    = suballocator->subbufs + last_id;

                  ids[avail_idx] = last_id;   // move id
                  last->idx      = avail_idx; // update idx[]
                }

              assert(suballocator->rem.avail > 0);

              // return origin
              return avail->origin;
            }
          else if ((avail->size > size_ru) && (spare_rem > 0)) // requested is less than available so split it
            {
              suballocator->total += size_ru;

              skc_uint                  spare_idx = suballocator->count - spare_rem;
              skc_subbuf_id_t     const spare_id  = ids[spare_idx];
              struct skc_subbuf * const spare     = suballocator->subbufs + spare_id;

              assert(spare->inuse == 0);

              // simple -- we're popping the top-of-stack of spares
              suballocator->rem.spare -= 1;

              // return id
              *subbuf_id = spare_id;

              SKC_SUBALLOCATOR_DEBUG_ALLOC(suballocator,spare_id,size_ru);

              // get prev
              struct skc_subbuf * const prev = avail->prev;

              if (prev != NULL)
                prev->next = spare;

              // init spare
              spare->prev    = prev;
              spare->next    = avail;
              spare->size    = size_ru;
              spare->origin  = avail->origin;
              spare->idx     = SKC_UINT_MAX; // defensive
              spare->inuse  += 1;

              // update curr
              avail->prev    = spare;
              avail->size   -= size_ru;
              avail->origin += size_ru;

              assert(suballocator->rem.avail > 0);

              return spare->origin;
            }
        }

      // uh oh... couldn't find enough memory
      skc_scheduler_wait(scheduler);
    }
}

//
// FIXME -- simplify this with a merge-with-prev() primitive
//

void
skc_suballocator_subbuf_free(struct skc_suballocator * const suballocator,
                             skc_subbuf_id_t                 subbuf_id)
{
  // get subbuf for id
  struct skc_subbuf * const subbuf = suballocator->subbufs + subbuf_id;

  assert(subbuf->inuse == 1);

  suballocator->total -= subbuf->size;

  SKC_SUBALLOCATOR_DEBUG_FREE(suballocator,subbuf_id,subbuf->size);

  //
  // try to merge subbuf with left and maybe right and then dispose
  //
  struct skc_subbuf * prev;
  struct skc_subbuf * next;

  if (((prev = subbuf->prev) != NULL) && !prev->inuse)
    {
      next = subbuf->next;

      if ((next != NULL) && !next->inuse)
        {
          subbuf->inuse -= 1;

          assert(next->inuse == 0);

          // increment size
          prev->size += (subbuf->size + next->size);

          struct skc_subbuf * const nextnext = next->next;

          // update next link
          prev->next = nextnext;

          // update prev link
          if (nextnext != NULL)
            nextnext->prev = prev;

          //
          // both subbuf and next are now spare which means we need to
          // move final available subbuffer into next's old position
          // unless they're the same
          //
          skc_uint const last_idx = --suballocator->rem.avail;
          skc_uint const next_idx = next->idx;

          assert(suballocator->rem.avail > 0);

          if (last_idx != next_idx)
            {
              skc_subbuf_id_t     const last_id = suballocator->ids[last_idx];
              struct skc_subbuf * const last    = suballocator->subbufs + last_id;

              suballocator->ids[next_idx]       = last_id;
              last->idx                         = next_idx;
            }

          skc_subbuf_id_t  const next_id   = (skc_subbuf_id_t)(next - suballocator->subbufs);

          skc_uint         const spare_rem = suballocator->rem.spare + 2;
          skc_uint         const spare_idx = suballocator->count - spare_rem;

          suballocator->rem.spare          = spare_rem;
          suballocator->ids[spare_idx + 0] = subbuf_id;
          suballocator->ids[spare_idx + 1] = next_id;
        }
      else
        {
          prev->size += subbuf->size;
          prev->next  = next;

          if (next != NULL)
            next->prev = prev;

          subbuf->inuse -= 1;

          assert(subbuf->inuse == 0);
          assert(suballocator->rem.avail > 0);

          suballocator->ids[suballocator->count - ++suballocator->rem.spare] = subbuf_id;
        }
    }
  //
  // try to merge with right
  //
  else if (((next = subbuf->next) != NULL) && !next->inuse)
    {
      subbuf->inuse -= 1;

      assert(subbuf->inuse == 0);
      assert(suballocator->rem.avail > 0);

      next->prev     = prev;
      next->origin   = subbuf->origin;
      next->size    += subbuf->size;

      if (prev != NULL)
        prev->next = next;

      // subbuf is now spare
      suballocator->ids[suballocator->count - ++suballocator->rem.spare] = subbuf_id;
    }
  else // couldn't merge with a neighbor
    {
      skc_uint avail_idx = suballocator->rem.avail++;

      // subbuf is now available
      subbuf->idx    = avail_idx;
      subbuf->inuse -= 1;

      assert(subbuf->inuse == 0);
      assert(suballocator->rem.avail > 0);

      suballocator->ids[avail_idx] = subbuf_id;
    }
}

//
//
//

#if 0

//
// At some point there might be a reason to sort the available
// subbuffers into some useful order -- presumably to binary search
// for the closest match or to chip away at the largest available
// subbuffer
//

static
void
skc_suballocator_optimize(struct skc_suballocator * const suballocator)
{
  ;
}

#endif

//
//
//
