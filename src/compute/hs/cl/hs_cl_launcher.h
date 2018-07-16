/*
 * Copyright 2016 Google Inc.
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
#include <stdint.h>
#include <stdbool.h>

//
//
//

#include "hs_cl_target.h"

//
//
//

struct hs_cl *
hs_cl_create(struct hs_cl_target const * const target,
             cl_context                        context,
             cl_device_id                      device_id);


//
//
//

void
hs_cl_release(struct hs_cl * const hs);

//
// Determine what padding will be applied to the input and output
// buffers.
//
// Always check to see if the allocated buffers are large enough.
//
// count                    : number of keys
// count + count_padded_in  : additional keys required for sorting
// count + count_padded_out : additional keys required for merging
//

void
hs_cl_pad(struct hs_cl const * const hs,
          uint32_t             const count,
          uint32_t           * const count_padded_in,
          uint32_t           * const count_padded_out);

//
// Sort the keys in the vin buffer and store them in the vout buffer.
//
// If vout is NULL then the sort will be performed in place.
//
// The implementation assumes the command queue is out-of-order.
//

void
hs_cl_sort(struct hs_cl const * const hs,
           cl_command_queue           cq,
           uint32_t             const wait_list_size,
           cl_event           *       wait_list,
           cl_event           *       event,
           cl_mem                     vin,
           cl_mem                     vout,
           uint32_t             const count,
           uint32_t             const count_padded_in,
           uint32_t             const count_padded_out,
           bool                 const linearize);

//
//
//
