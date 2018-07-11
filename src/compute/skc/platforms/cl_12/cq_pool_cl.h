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
  cl_command_queue          * cq;
  cl_command_queue_properties cq_props;

  skc_uint                    size;
  skc_uint                    reads;
  skc_uint                    writes;
};

//l
//
//

void
skc_cq_pool_create(struct skc_runtime        * const runtime,
                   struct skc_cq_pool        * const pool,
                   cl_command_queue_properties const cq_props,
                   skc_uint                    const size);

void
skc_cq_pool_dispose(struct skc_runtime * const runtime,
                    struct skc_cq_pool *       pool);

//
//
//
