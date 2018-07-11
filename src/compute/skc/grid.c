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

#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "grid.h"
#include "macros.h"
#include "runtime_cl_12.h"

//
// SKC grid dependencies can be represented with a DAG.
//
// This dependency graph may be modified to include some sort of block
// pool barrier to make block recovery explicit (and guaranteed safe).
//
//
//              PATH BUILDER
//                    |
//                    |
//                    |
//                    v
//             RASTER BUILDER
//                    |
//            +----+  |           +----+
//    Set Ops |    |  |           |    | Set Ops
//            |    v  v           v    |
//            +--COMPOSITION  STYLING--+
//                    |          |
//                    | +--------+
//                    | |
//                    v v
//                  SURFACE
//
//
//       STAGE                DEPENDENCIES
//  ==============    ============================
//  PATH BUILDER      -
//  RASTER BUILDER    PATH BUILDER
//  COMPOSITION       RASTER BUILDER, *COMPOSITION
//  STYLING           -, *STYLING
//  SURFACE           COMPOSITION, STYLING
//

//
// How many active grids can/should we have?
//
// FIXME -- we'll need to provide a small level of indirection if we
// want to support a much larger number of work-in-progress grids
//
// For now and for simplicity, unify all grid ids in one set.
//

typedef skc_uchar            skc_grid_id_t;  // 256 values
#define SKC_GRID_ID_INVALID  SKC_UCHAR_MAX   // 255

#define SKC_GRID_SIZE_IDS    (SKC_GRID_ID_INVALID-1)
#define SKC_GRID_SIZE_WORDS  ((SKC_GRID_SIZE_IDS+31)/32)

//
//
//

typedef enum skc_grid_state_e {

  SKC_GRID_STATE_READY,
  SKC_GRID_STATE_WAITING,
  SKC_GRID_STATE_FORCED,
  SKC_GRID_STATE_EXECUTING,
  SKC_GRID_STATE_COMPLETE,
  SKC_GRID_STATE_DETACHED,

  SKC_GRID_STATE_COUNT

} skc_grid_state_e;

//
//
//

struct skc_grid_pfn_name
{
  skc_grid_pfn pfn;
  char const * name;
};

//
//
//

struct skc_grid
{
  skc_grid_state_e          state;
  skc_uint                  id;

  struct skc_grid_deps    * deps;    // backpointer to deps
  void                  * * addr;    // pointer to invalidate

  void                    * data;

  struct skc_grid_pfn_name  waiting; // optional - if defined, typically used to yank the grid away from host
  struct skc_grid_pfn_name  execute; // optional - starts execution of waiting grid
  struct skc_grid_pfn_name  dispose; // optional - invoked when grid is complete

  struct {
    skc_uint                words[SKC_GRID_SIZE_WORDS]; // 0:inactive, 1:active
    skc_uint                count;
  } before;

  struct {
    skc_uint                words[SKC_GRID_SIZE_WORDS]; // 0:inactive, 1:active
    skc_uint                count;
  } after;
};

//
//
//

struct skc_grid_deps
{
  struct skc_runtime   * runtime;
  struct skc_scheduler * scheduler;

  skc_grid_id_t        * handle_map;

  struct skc_grid        grids [SKC_GRID_SIZE_IDS];   // deps + pfns + data
  skc_uint               active[SKC_GRID_SIZE_WORDS]; // 1:inactive, 0:active

  skc_uint               count;                       // number of active ids
};

//
//
//

static
void
skc_grid_call(skc_grid_t const grid, struct skc_grid_pfn_name const * const pn)
{
  if (pn->pfn != NULL) {
    pn->pfn(grid);
  }
}

static
void
skc_grid_schedule(skc_grid_t const grid, struct skc_grid_pfn_name const * const pn)
{
  if (pn->pfn != NULL) {
    skc_scheduler_schedule(grid->deps->scheduler,pn->pfn,grid,pn->name);
  }
}

//
//
//

static
void
skc_grid_invalidate(skc_grid_t const grid)
{
  if (grid->addr != NULL) {
    *grid->addr = NULL;
  }
}

//
//
//

#if 0
skc_grid_t
skc_grid_move(skc_grid_t         const grid,
              skc_grid_state_e * const state,
              skc_grid_t       * const addr,
              void             * const data)
{
  skc_grid_invalidate(grid);

  grid->state = state;
  grid->addr  = addr;
  grid->data  = data;

  return grid;
}
#endif

void *
skc_grid_get_data(skc_grid_t const grid)
{
  return grid->data;
}

void
skc_grid_set_data(skc_grid_t const grid, void * const data)
{
  grid->data = data;
}

//
//
//

skc_grid_deps_t
skc_grid_deps_create(struct skc_runtime   * const runtime,
                     struct skc_scheduler * const scheduler,
                     skc_uint               const handle_pool_size)
{
  struct skc_grid_deps * const deps = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*deps));

  // save runtime
  deps->runtime    = runtime;
  deps->scheduler  = scheduler;

  size_t const handle_map_size = sizeof(*deps->handle_map) * handle_pool_size;

  // allocate handle map
  deps->handle_map = skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,handle_map_size);

  // initialize handle map
  memset(deps->handle_map,0xFF,handle_map_size);

  // grids
  struct skc_grid * const grids = deps->grids;

#if 0 // DELETE ME LATER
  // initalize ids once -- could always infer id using offsetof()
  for (skc_uint id=0; id < SKC_GRID_SIZE_IDS; id++)
    grids[id].id = id;
#endif

  // mark all grids inactive except for last bit -- 1:inactive / 0:active
  for (skc_uint ii=0; ii < SKC_GRID_SIZE_WORDS-1; ii++)
    deps->active[ii] = 0xFFFFFFFF;

  // last bit is marked active so that it is never allocated
  deps->active[SKC_GRID_SIZE_WORDS-1] = 0x7FFFFFFF;

  // nothing active
  deps->count = 1;

  return deps;
}

void
skc_grid_deps_dispose(skc_grid_deps_t const deps)
{
  //
  // FIXME -- debug checks for active grids
  //
  skc_runtime_host_perm_free(deps->runtime,deps->handle_map);
  skc_runtime_host_perm_free(deps->runtime,deps);
}

//
//
//

#ifndef NDEBUG

void
skc_grid_deps_debug(struct skc_grid_deps const * const deps)
{
  fprintf(stderr,
          "00000000000000001111111111111111\n"
          "0123456789ABCDEF0123456789ABCDEF\n"
          "--------------------------------\n");

  for (skc_uint ii=0; ii<SKC_GRID_SIZE_WORDS; ii++)
    {
      skc_uint const a = deps->active[ii];
      fprintf(stderr,
              "%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u"
              "%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u%1u\n",
              (a>>0x00)&1,(a>>0x01)&1,(a>>0x02)&1,(a>>0x03)&1,
              (a>>0x04)&1,(a>>0x05)&1,(a>>0x06)&1,(a>>0x07)&1,
              (a>>0x08)&1,(a>>0x09)&1,(a>>0x0A)&1,(a>>0x0B)&1,
              (a>>0x0C)&1,(a>>0x0D)&1,(a>>0x0E)&1,(a>>0x0F)&1,
              (a>>0x10)&1,(a>>0x11)&1,(a>>0x12)&1,(a>>0x13)&1,
              (a>>0x14)&1,(a>>0x15)&1,(a>>0x16)&1,(a>>0x17)&1,
              (a>>0x18)&1,(a>>0x19)&1,(a>>0x1A)&1,(a>>0x1B)&1,
              (a>>0x1C)&1,(a>>0x1D)&1,(a>>0x1E)&1,(a>>0x1F)&1);
    }

  fprintf(stderr,"\n");
}

#endif

//
//
//

skc_grid_t
skc_grid_deps_attach(skc_grid_deps_t const deps,
                     skc_grid_t    * const addr,
                     void          * const data,
                     skc_grid_pfn          waiting_pfn,  // upon READY         > WAITING
                     skc_grid_pfn          execute_pfn,  // upon READY/WAITING > EXECUTING
                     skc_grid_pfn          dispose_pfn,  // upon EXECUTING     > COMPLETE
                     char    const * const waiting_name,
                     char    const * const execute_name,
                     char    const * const dispose_name)
{
  //
  // FIXME -- no more ids -- either fatal or flush & wait for grids to be released
  //
  // assert(deps->count < SKC_GRID_SIZE_IDS);
  //
  while (deps->count == SKC_GRID_SIZE_IDS)
    skc_scheduler_wait_one(deps->scheduler);

  // otherwise, an id exists so decrement count
  deps->count += 1;

  // find first set bit (1:inactive)
  skc_uint * active = deps->active;
  skc_uint   first  = 0;

  while (1)
    {
      skc_uint const idx = SKC_LZCNT_32(*active);

      first += idx;

      if (idx < 32)
        {
          // make inactive bit active: 1 -> 0
          *active &= ~(0x80000000 >> idx); // 0:active
          break;
        }

      // otherwise, inspect next word for inactive bit
      active += 1;
    }

  struct skc_grid * const grid = deps->grids + first;

  // save grid pointer
  if (addr != NULL)
    *addr = grid;

  // initialize elem
  *grid = (struct skc_grid){
    .state   = SKC_GRID_STATE_READY,
    .id      = first,
    .deps    = deps,
    .addr    = addr,
    .data    = data,
    .waiting = { .pfn = waiting_pfn, .name = waiting_name },
    .execute = { .pfn = execute_pfn, .name = execute_name },
    .dispose = { .pfn = dispose_pfn, .name = dispose_name },
    .before  = { { 0 }, 0 },
    .after   = { { 0 }, 0 }
  };

  return grid;
}

//
//
//

static
skc_bool
skc_grid_words_set(skc_uint ids[SKC_GRID_SIZE_WORDS], skc_uint const id)
{
  skc_uint * const ptr  = ids + (id/32);
  skc_uint   const pre  = *ptr;
  skc_uint   const post = pre | (0x80000000 >> (id & 0x1F)); // set

  *ptr = post;

  return pre != post;
}

static
skc_bool
skc_grid_words_clear(skc_uint ids[SKC_GRID_SIZE_WORDS], skc_uint const id)
{
  skc_uint * const ptr  = ids + (id/32);
  skc_uint   const pre  = *ptr;
  skc_uint   const post = pre & ~(0x80000000 >> (id & 0x1F)); // clear

  *ptr = post;

  return pre != post;
}

//
// we may want to allow the host to detach a grid
//

static
void
skc_grid_detach(skc_grid_t const grid)
{
  // for now make sure grid is complete
  // assert(grid->state == SKC_GRID_STATE_COMPLETE);

  // transition state
  grid->state = SKC_GRID_STATE_DETACHED;

  //
  // FIXME -- save profiling info
  //

  // cleanup
  if (skc_grid_words_set(grid->deps->active,grid->id)) // 1:inactive
    grid->deps->count -= 1;
}

//
//
//

void
skc_grid_map(skc_grid_t const grid, skc_handle_t const handle)
{
  grid->deps->handle_map[handle] = grid->id;
}

//
//
//

void
skc_grid_deps_force(skc_grid_deps_t      const deps,
                    skc_handle_t const * const handles,
                    skc_uint             const count)
{
  //
  // FIXME -- test to make sure handles aren't completely out of range integers
  //
  skc_grid_id_t * const handle_map = deps->handle_map;

  for (skc_uint ii=0; ii<count; ii++)
    {
      skc_grid_id_t grid_id = handle_map[SKC_TYPED_HANDLE_TO_HANDLE(handles[ii])];

      if (grid_id < SKC_GRID_ID_INVALID)
        {
          skc_grid_t const grid = deps->grids + grid_id;

          skc_grid_force(grid);

          while (grid->state >= SKC_GRID_STATE_COMPLETE)
            skc_scheduler_wait_one(deps->scheduler);
        }
    }
}

void
skc_grid_deps_unmap(skc_grid_deps_t      const deps,
                    skc_handle_t const * const handles,
                    skc_uint             const count)
{
  skc_grid_id_t * const handle_map = deps->handle_map;

  for (skc_uint ii=0; ii<count; ii++)
    handle_map[handles[ii]] = SKC_GRID_ID_INVALID;
}

//
// NOTE: We want this routine to be very very fast. The array of bit
// flags is probably as fast as we can go for a modest number of
// grids.
//
// NOTE: The before grid should never be NULL.  This means the grid's
// lifecycle should match the lifetime of the object it represents.
// This also means the grid "invalidation upon start" feature should
// be well understood before using it to clear the skc_grid_t.
//

void
skc_grid_happens_after_grid(skc_grid_t const after,
                            skc_grid_t const before)
{
  // declarations can't be made on non-ready grids
  assert(after->state == SKC_GRID_STATE_READY);

  if (before->state >= SKC_GRID_STATE_COMPLETE)
    return;

  if (skc_grid_words_set(after->before.words,before->id))
    after->before.count += 1;

  if (skc_grid_words_set(before->after.words,after->id))
    before->after.count += 1;
}

void
skc_grid_happens_after_handle(skc_grid_t const after, skc_handle_t const before)
{
  assert(after->state == SKC_GRID_STATE_READY);

  skc_uint const id_before = after->deps->handle_map[before];

  if (id_before >= SKC_GRID_ID_INVALID)
    return;

  if (skc_grid_words_set(after->before.words,id_before))
    after->before.count += 1;

  skc_grid_t const grid_before = after->deps->grids + id_before;

  if (skc_grid_words_set(grid_before->after.words,after->id))
    grid_before->after.count += 1;
}

//
// Remove dependency from grid
//

static
void
skc_grid_clear_dependency(skc_grid_t const after, skc_uint const before)
{
  skc_bool const is_change = skc_grid_words_clear(after->before.words,before);

  assert(is_change); // for now let's make sure this is a rising edge

  after->before.count -= 1;

  if ((after->before.count == 0) && ((after->state == SKC_GRID_STATE_WAITING) ||
                                     (after->state == SKC_GRID_STATE_FORCED)))
    {
      // schedule grid for execution
      after->state = SKC_GRID_STATE_EXECUTING;
      skc_grid_schedule(after,&after->execute);
    }
}

//
// Start the ready grid and wait for dependencies to complete
//

void
skc_grid_start(skc_grid_t const grid)
{
  // nothing to do if this grid isn't in a ready state
  if (grid->state != SKC_GRID_STATE_READY)
    return;

  // record transition through waiting state
  grid->state = SKC_GRID_STATE_WAITING;

  // the waiting pfn may be null -- e.g. the path builder
  // skc_grid_schedule(grid,&grid->waiting);
  skc_grid_call(grid,&grid->waiting);

  // clear the reference
  skc_grid_invalidate(grid);

  // execute if there are no dependencies
  if (grid->before.count == 0)
    {
      // tell grid it can execute
      grid->state = SKC_GRID_STATE_EXECUTING;
      skc_grid_schedule(grid,&grid->execute);
    }
}

//
// Start this grid and all its ready dependencies
//

void
skc_grid_force(skc_grid_t const grid)
{
  // return if this grid was forced, executing or complete
  if (grid->state >= SKC_GRID_STATE_FORCED)
    return;

  // if ready then move to waiting state
  if (grid->state == SKC_GRID_STATE_READY)
    {
      // tell grid to wait for execution
      grid->state = SKC_GRID_STATE_WAITING;

      // the waiting pfn may be null -- e.g. the path builder
      // skc_grid_schedule(grid,&grid->waiting);
      skc_grid_call(grid,&grid->waiting);

      // clear the reference
      skc_grid_invalidate(grid);
    }

  skc_uint before_count = grid->before.count;

  // if there are no grid dependencies then execute
  if (before_count == 0)
    {
      // tell grid it can execute
      grid->state = SKC_GRID_STATE_EXECUTING;
      skc_grid_schedule(grid,&grid->execute);
    }
  else // otherwise, start or make waiting all dependencies
    {
      grid->state = SKC_GRID_STATE_FORCED;

      struct skc_grid * before       = grid->deps->grids;
      skc_uint        * before_words = grid->before.words;
      skc_uint          active       = *before_words++;

      while (true)
        {
          // find first active
          skc_uint const idx = SKC_LZCNT_32(active);

          // no bits set so inspect next word
          if (idx == 32)
            {
              active  = *before_words++;
              before += 1;
              continue;
            }
          else // clear active
            {
              active       &= ~(0x80000000 >> idx);
              before_count -= 1;
            }

          // otherwise, force this elem with dependent
          skc_grid_force(before+idx);

          // no more bits?
          if (before_count == 0)
            break;
        }
    }
}

//
// Notify grids dependent on this grid that this grid is complete
//

void
skc_grid_complete(skc_grid_t const grid)
{
  // debug: grid was executing
  assert(grid->state == SKC_GRID_STATE_EXECUTING);

  // move grid to completion and dispose after notifying dependents
  grid->state = SKC_GRID_STATE_COMPLETE;

  skc_uint after_count = grid->after.count;

  if (after_count > 0)
    {
      // find set bits
      struct skc_grid * after       = grid->deps->grids;
      skc_uint        * after_words = grid->after.words;
      skc_uint          active      = *after_words++;

      while (true)
        {
          // find first active
          skc_uint const idx = SKC_LZCNT_32(active);

          // no bits set so inspect next word
          if (idx == 32)
            {
              active  = *after_words++;
              after  += 32;
              continue;
            }
          else // clear active
            {
              active      &= ~(0x80000000 >> idx);
              after_count -= 1;
            }

          // otherwise, clear this dependency
          skc_grid_clear_dependency(after+idx,grid->id);

          // no more bits?
          if (after_count == 0)
            break;
        }
    }

  // dispose of resources
  skc_grid_call(grid,&grid->dispose);

  // we don't need to hang on to this grid id any longer
  skc_grid_detach(grid);
}

///////////////////////////////////////////////////////////////////////////
//
// ALTERNATIVE IMPLEMENTATION WOULD SUPPORT A VARIABLE NUMBER OF
// ACTIVE GRIDS PER STAGE PRESUMABLY RESULTING IN SLIGHTLY LESS MEMORY
// USAGE.
//
// THE #1 OBJECTIVE OF THE GRID IMPLEMENTATION IS TO ENSURE THAT THE
// "HAPPENS-BEFORE" DECLARATION REMAINS ***FAST*** SINCE IT WILL BE
// CALLED FREQUENTLY.  THUS THE |GRIDS|^2 BIT ARRAY...
//
// WE DON'T NEED THIS RIGHT NOW...
//

#if 0

//
// For now, make them all the same size
//

#define SKC_GRID_STAGE_WORDS_PATH_BUILDER          SKC_GRID_MASK_WORDS
#define SKC_GRID_STAGE_WORDS_RASTER_BUILDER        SKC_GRID_MASK_WORDS
#define SKC_GRID_STAGE_WORDS_COMPOSITION           SKC_GRID_MASK_WORDS
#define SKC_GRID_STAGE_WORDS_STYLING               SKC_GRID_MASK_WORDS
#define SKC_GRID_STAGE_WORDS_SURFACE_COMPOSITION   SKC_GRID_MASK_WORDS
#define SKC_GRID_STAGE_WORDS_SURFACE_STYLING       SKC_GRID_MASK_WORDS

//
//
//

typedef enum skc_grid_stage_type {

  SKC_GRID_STAGE_TYPE_PATH_BUILDER,
  SKC_GRID_STAGE_TYPE_RASTER_BUILDER,
  SKC_GRID_STAGE_TYPE_COMPOSITION,
  SKC_GRID_STAGE_TYPE_STYLING,
  SKC_GRID_STAGE_TYPE_SURFACE_COMPOSITION,
  SKC_GRID_STAGE_TYPE_SURFACE_STYLING,

  SKC_GRID_STAGE_TYPE_COUNT

} skc_grid_stage_type;

//
//
//

union skc_grid_id
{
  skc_grid_id_t u32;

  struct {
    skc_ushort  index;
    skc_ushort  stage;
  };
}

SKC_STATIC_ASSERT(sizeof(union skc_grid_id) == sizeof(skc_uint));

//
//
//

#endif

//
//
//
