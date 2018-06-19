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
// Returns some useful info about algorithm's configuration for the
// target architecture.
//

struct hs_info
{
  uint32_t words; // words-per-key (1 = uint, 2 = ulong)
  uint32_t keys;  // keys-per-lane
  uint32_t lanes; // lanes-per-warp
};

//
//
//

void
hs_create(cl_context             context,
          cl_device_id           device_id,
          struct hs_info * const info);

//
//
//

void
hs_release();

//
// Size the buffers.
//

void
hs_pad(uint32_t   const count,
       uint32_t * const count_padded_in,
       uint32_t * const count_padded_out);

//
// Sort the keys in the vin buffer and store them in the vout buffer.
//
// The vin and vout buffers can be the same buffer.
//
// If it is necessary, a barrier should be enqueued before running
// hs_sort().
//
// A final barrier will enqueued before returning.
//

void
hs_sort(cl_command_queue cq, // out-of-order cq
        cl_mem           vin,
        cl_mem           vout,
        uint32_t   const count,
        uint32_t   const count_padded_in,
        uint32_t   const count_padded_out,
        bool       const linearize);

//
//
//
