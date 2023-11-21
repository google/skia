/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanMutableTextureStatePriv_DEFINED
#define skgpu_VulkanMutableTextureStatePriv_DEFINED

#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstdint>

namespace skgpu {
class MutableTextureState;
class VulkanMutableTextureState;
}

namespace skgpu::MutableTextureStates {
    const VulkanMutableTextureState& GetVulkanState(const MutableTextureState& state);
    const VulkanMutableTextureState& GetVulkanState(const MutableTextureState* state);
    void SetVkImageLayout(MutableTextureState* state, VkImageLayout layout);
    void SetVkQueueFamilyIndex(MutableTextureState* state, uint32_t queueFamilyIndex);
}

// TODO(b/293490566) Move VulkanMutableTextureState out of include/private/gpu/vk/VulkanTypesPriv.h
// and into here when the type union is removed from MutableTextureState
#endif
