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

void
vk_pipeline_cache_create(VkDevice                            device,
                         VkAllocationCallbacks const *       allocator,
                         char                  const * const name,
                         VkPipelineCache             *       pipeline_cache);

void
vk_pipeline_cache_destroy(VkDevice                            device,
                          VkAllocationCallbacks const *       allocator,
                          char                  const * const name,
                          VkPipelineCache                     pipeline_cache);

//
//
//
