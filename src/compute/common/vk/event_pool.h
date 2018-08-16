/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

struct vk_event_pool *
vk_event_pool_create(VkDevice device, VkAllocationCallbacks const * allocator, uint32_t const resize);

void
vk_event_pool_release(struct vk_event_pool * const event_pool);

void
vk_event_pool_reset(struct vk_event_pool * const event_pool);

VkEvent
vk_event_pool_acquire(struct vk_event_pool * const event_pool);

//
//
//
