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

#include <vulkan/vulkan.h>

//
//
//

#include <stdint.h>
#include <stdbool.h>

//
//
//

#include "hs_spirv_target.h"

//
//
//

struct hs_vk *
hs_vk_create(struct hs_spirv_target const * const target,
             VkDevice                             device,
             VkAllocationCallbacks  const *       allocator,
             VkPipelineCache                      pipeline_cache);

//
// Resources will be disposed of with the same device and allocator
// used for creation.
//

void
hs_vk_release(struct hs_vk * const hs);

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
hs_vk_pad(struct hs_vk const * const hs,
          uint32_t             const count,
          uint32_t           * const count_padded_in,
          uint32_t           * const count_padded_out);

//
// Sort the keys in the vin buffer and store them in the vout buffer.
//
// If vout is NULL then the sort will be performed in place.
//

#if 0
void
hs_vk_sort(struct hs_vk const * const hs,
           vk_command_queue           cq,
           uint32_t             const wait_list_size,
           vk_event           *       wait_list,
           vk_event           *       event,
           vk_mem                     vin,
           vk_mem                     vout,
           uint32_t             const count,
           uint32_t             const count_padded_in,
           uint32_t             const count_padded_out,
           bool                 const linearize);
#endif

//
//
//
