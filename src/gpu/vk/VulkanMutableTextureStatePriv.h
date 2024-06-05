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
}

namespace skgpu::MutableTextureStates {
    void SetVkImageLayout(MutableTextureState* state, VkImageLayout layout);
    void SetVkQueueFamilyIndex(MutableTextureState* state, uint32_t queueFamilyIndex);
}

#endif
