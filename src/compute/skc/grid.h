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

#include "handle.h"
#include "scheduler.h"

//
// The requirement is that every grid struct begin with an skc_grid_t
//

typedef struct skc_grid      * skc_grid_t;
typedef struct skc_grid_deps * skc_grid_deps_t;

//
//
//

typedef void (* skc_grid_pfn)(skc_grid_t const grid);

//
//
//

#define SKC_IS_GRID_INVALID(grid)  (grid == NULL)

//
//
//

#define SKC_GRID_DEPS_ATTACH(deps,addr,data,waiting_pfn,execute_pfn,dispose_pfn) \
  skc_grid_deps_attach(deps,addr,data,                                  \
                       waiting_pfn,execute_pfn,dispose_pfn,             \
                       #waiting_pfn,#execute_pfn,#dispose_pfn)          \
//
//
//

skc_grid_deps_t
skc_grid_deps_create(struct skc_runtime   * const runtime,
                     struct skc_scheduler * const scheduler,
                     skc_uint               const handle_pool_size);

void
skc_grid_deps_dispose(skc_grid_deps_t const deps);

//
//
//

#ifndef NDEBUG
void
skc_grid_deps_debug(struct skc_grid_deps const * const deps);
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
                     char    const * const dispose_name);

#if 0
//
// Not using this yet -- needs to properly detach and reclaim a ready
// grid's resources
//
void
skc_grid_detach(skc_grid_t const grid);
#endif

//
//
//

void *
skc_grid_get_data(skc_grid_t const grid);

void
skc_grid_set_data(skc_grid_t const grid, void * const data);

//
//
//

void
skc_grid_map(skc_grid_t const grid, skc_handle_t const handle);

//
//
//

void
skc_grid_deps_force(skc_grid_deps_t      const deps,
                    skc_handle_t const * const handles,
                    skc_uint             const count);

void
skc_grid_deps_unmap(skc_grid_deps_t      const deps,
                    skc_handle_t const * const handles,
                    skc_uint             const count);

//
//
//

void
skc_grid_happens_after_grid(skc_grid_t const after,
                            skc_grid_t const before);

void
skc_grid_happens_after_handle(skc_grid_t   const after,
                              skc_handle_t const before);

//
// should be called by host
//

void
skc_grid_start(skc_grid_t const grid);

void
skc_grid_force(skc_grid_t const grid);

//
// should be called by the scheduler
//

void
skc_grid_complete(skc_grid_t const grid);

//
//
//

#if 0
//
// delete when ready
//
skc_grid_t
skc_grid_move(skc_grid_t         const grid,
              skc_grid_state_e * const state,
              skc_grid_t       * const addr,
              void             * const data);
#endif

//
//
//
