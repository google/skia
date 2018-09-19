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

#include "src/compute/skc/platforms/cl_12/block_pool_cl.h"
#include "src/compute/skc/platforms/cl_12/extent_cl_12.h"

//
// device side block pool
//

struct skc_block_pool
{
  union  skc_block_pool_size const * size;

  struct skc_extent_pdrw             blocks;
  struct skc_extent_pdrw             ids;
  struct skc_extent_phr_pdrw         atomics;
};

//
//
//
