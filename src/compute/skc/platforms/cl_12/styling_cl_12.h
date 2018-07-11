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

#include "styling.h"
#include "grid.h"
#include "extent_cl_12.h"
#include "assert_state.h"

//
// styling states
//

typedef enum skc_styling_state_e {

  SKC_STYLING_STATE_UNSEALING,
  SKC_STYLING_STATE_UNSEALED,
  SKC_STYLING_STATE_SEALING,
  SKC_STYLING_STATE_SEALED

} skc_styling_state_e;

//
// IMPL
//

struct skc_styling_impl
{
  struct skc_styling         * styling;
  struct skc_runtime         * runtime;

  SKC_ASSERT_STATE_DECLARE(skc_styling_state_e);

  skc_int                      lock_count;  // # of wip renders

  skc_grid_t                   grid;

  // in-order command queue
  cl_command_queue             cq;

  //
  // only 3 extents
  //
  struct skc_extent_phwN_pdrN  layers;
  struct skc_extent_phwN_pdrN  groups;
  struct skc_extent_phwN_pdrN  extras;
};

//
// ONLY VISIBLE WITHIN THIS RUNTIME
//

void
skc_styling_retain_and_lock(struct skc_styling * const styling);

void
skc_styling_unlock_and_release(struct skc_styling * const styling);

//
//
//
