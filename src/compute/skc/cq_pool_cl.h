/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

#include "types.h"

//
// Why we need to wrap command queue creation:
//
// - command queue creation is expensive
// 
// - the CL 1.2 function is deprecated in 2.0
//

struct skc_cq_pool
{
  skc_cq_type_e      type;
  skc_uint           size;
  skc_uint           reads;
  skc_uint           writes;
  cl_command_queue * cq;
};

//l
//
//

void
skc_cq_pool_create(struct skc_runtime * const runtime,
                   struct skc_cq_pool * const pool,
                   skc_uint             const type,
                   skc_uint             const size);

void
skc_cq_pool_dispose(struct skc_runtime * const runtime,
                    struct skc_cq_pool *       pool);

//
//
//
