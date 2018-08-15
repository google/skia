/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include <cuda.h>
#include <stdint.h>
#include <stdbool.h>

//
// Info about the algorithm configuration.
//

void
hs_cuda_info_u32(uint32_t * const key_words,
                 uint32_t * const val_words,
                 uint32_t * const slab_height,
                 uint32_t * const slab_width_log2);

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
hs_cuda_pad_u32(uint32_t   const count,
                uint32_t * const count_padded_in,
                uint32_t * const count_padded_out);

//
// Sort the keys in the vin buffer and store them in the vout buffer.
//
// If vout is NULL then the sort will be performed in place.
//
// The implementation assumes the command queue is out-of-order.
//

void
hs_cuda_sort_u32(uint32_t * const vin,
                 uint32_t * const vout,
                 uint32_t   const count,
                 uint32_t   const count_padded_in,
                 uint32_t   const count_padded_out,
                 bool       const linearize,
                 cudaStream_t     stream0,  // primary stream
                 cudaStream_t     stream1,  // auxilary streams
                 cudaStream_t     stream2); // for concurrency

//
//
//
