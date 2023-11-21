/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanMutableTextureState_DEFINED
#define skgpu_VulkanMutableTextureState_DEFINED

#include "include/gpu/MutableTextureState.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstdint>

namespace skgpu::MutableTextureStates {
    MutableTextureState MakeVulkan(VkImageLayout layout, uint32_t queueFamilyIndex);
    VkImageLayout GetVkImageLayout(const MutableTextureState& state);
    VkImageLayout GetVkImageLayout(const MutableTextureState* state);
    uint32_t GetVkQueueFamilyIndex(const MutableTextureState& state);
    uint32_t GetVkQueueFamilyIndex(const MutableTextureState* state);
}

#endif
