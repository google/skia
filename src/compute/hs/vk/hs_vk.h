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

#include "hs_vk_target.h"

//
// Create a HotSort instance from a specific vendor, architecture and
// key/val size target.
//

struct hs_vk *
hs_vk_create(struct hs_vk_target   const * const target,
             VkDevice                            device,
             VkAllocationCallbacks const *       allocator,
             VkPipelineCache                     pipeline_cache);

//
// Resources will be disposed of with the same device and allocator
// used for creation.
//

void
hs_vk_release(struct hs_vk * const hs);

//
// Allocate a thread local descriptor set for the vin and vout
// VkBuffers.  Note that HotSort uses only one descriptor set.
//
// Don't forget to return the descriptor set back to the same pool
// with vkFreeDescriptorSets().
//

VkDescriptorSet
hs_vk_ds_alloc(struct hs_vk const * const hs, VkDescriptorPool desc_pool);

//
// Explicitly bind the descriptor set describing the 'vin' and 'vout'
// buffers to the command buffer before calling 'hs_vk_sort()'.
//
// If 'vout' is VK_NULL_HANDLE then the sort will be performed in
// place.
//
// FIXME -- do we want to expose a set index?
//
// FIXME -- do we want to allow specialization of the buffer bindings
// or sets?
//

void
hs_vk_ds_bind(struct hs_vk const * const hs,
              VkDescriptorSet            hs_ds,
              VkCommandBuffer            cb,
              VkBuffer                   vin,
              VkBuffer                   vout);

//
// Explicitly reveal what padding of maximum valued keys will be
// applied to the input and output buffers.
//
//   count            : input number of keys
//   count_padded_in  : adjusted count of keys in vin[] buffer
//   count_padded_out : adjusted count of keys in vout[] buffer
//
// Instead of implicitly padding the buffers, HotSort requires this
// explicit step to support use cases like:
//
//   - writing past the end of the vin[] buffer
//   - dynamically allocating a vout[] buffer before sorting
//

void
hs_vk_pad(struct hs_vk const * const hs,
          uint32_t             const count,
          uint32_t           * const count_padded_in,
          uint32_t           * const count_padded_out);

//
// Append commands to the command buffer that when enqueued will sort
// the keys in the 'vin' buffer and store them in the 'vout' buffer.
//
// If 'vout' is VK_NULL_HANDLE then the sort will be performed in
// place.
//
// Pipeline barriers should be applied both before and after invoking
// this function.
//
// Note that the algorithm *may* perform transfer operations on the
// buffers before executing a compute shader.
//
// The algorithm ends with a single compute shader.
//

void
hs_vk_sort(struct hs_vk const * const hs,
           VkCommandBuffer            cb,
           VkBuffer                   vin,
           VkPipelineStageFlags const vin_src_stage,
           VkAccessFlagBits     const vin_src_access,
           VkBuffer                   vout,
           VkPipelineStageFlags const vout_src_stage,
           VkAccessFlagBits     const vout_src_access,
           uint32_t             const count,
           uint32_t             const count_padded_in,
           uint32_t             const count_padded_out,
           bool                 const linearize);

//
//
//
