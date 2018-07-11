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

#include "composition.h"
#include "assert_state.h"
#include "grid.h"
#include "extent_cl_12.h"
#include "extent_ring.h"

//
// composition states
//

typedef enum skc_composition_state_e {

  SKC_COMPOSITION_STATE_UNSEALING,
  SKC_COMPOSITION_STATE_UNSEALED,
  SKC_COMPOSITION_STATE_SEALING,
  SKC_COMPOSITION_STATE_SEALED

} skc_composition_state_e;

//
// IMPL
//

struct skc_composition_impl
{
  struct skc_composition        * composition;
  struct skc_runtime            * runtime;

  SKC_ASSERT_STATE_DECLARE(skc_composition_state_e);

  skc_int                         lock_count; // wip renders

  struct {
    skc_grid_t                    place;
    skc_grid_t                    sort;
  } grids;

  cl_command_queue                cq;

  struct {
    cl_kernel                     place;
    cl_kernel                     segment;
  } kernels;

  // raster ids must be held until the composition is reset or
  // released and then their refcounts can be decremented
  struct {
    struct skc_extent_phrw        extent;
    skc_uint                      count;
  } saved;

  struct {
    struct skc_extent_ring        ring;   // how many slots left?
    struct skc_extent_phw1g_tdrNs extent; // wip command extent
  } cmds;

  // composition extent length
  struct skc_extent_phr_pdrw      atomics;

  // composition ttck extent
  struct skc_extent_pdrw          keys;

  // key offsets in sealed and sorted ttck extent
  struct skc_extent_pdrw          offsets;
};

//
// ATOMICS
//

struct skc_place_atomics
{
  skc_uint keys;
  skc_uint offsets;
};

//
// ONLY VISIBLE WITHIN THIS RUNTIME
//

void
skc_composition_retain_and_lock(struct skc_composition * const composition);

void
skc_composition_unlock_and_release(struct skc_composition * const composition);

//
//
//
