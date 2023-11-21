/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanMutableTextureState.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/vk/VulkanTypesPriv.h"
#include "src/gpu/vk/VulkanMutableTextureStatePriv.h"

namespace skgpu {
class MTSVKPriv {
public:
    static const VulkanMutableTextureState& VkState(const MutableTextureState& state) {
        return state.fVkState;
    }

    static const VulkanMutableTextureState& VkState(const MutableTextureState* state) {
        return state->fVkState;
    }

    static VulkanMutableTextureState& VkState(MutableTextureState* state) {
        return state->fVkState;
    }
};
}  // namespace skgpu

namespace skgpu::MutableTextureStates {

MutableTextureState MakeVulkan(VkImageLayout layout, uint32_t queueFamilyIndex) {
    return MutableTextureState(layout, queueFamilyIndex);
}

VkImageLayout GetVkImageLayout(const MutableTextureState& state) {
    SkASSERT(state.backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state).getImageLayout();
}

VkImageLayout GetVkImageLayout(const MutableTextureState* state) {
    SkASSERT(state);
    SkASSERT(state->backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state).getImageLayout();
}

void SetVkImageLayout(MutableTextureState* state, VkImageLayout layout) {
    SkASSERT(state->backend() == BackendApi::kVulkan);
    MTSVKPriv::VkState(state).setImageLayout(layout);
}

uint32_t GetVkQueueFamilyIndex(const MutableTextureState& state) {
    SkASSERT(state.backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state).getQueueFamilyIndex();
}

uint32_t GetVkQueueFamilyIndex(const MutableTextureState* state) {
    SkASSERT(state);
    SkASSERT(state->backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state).getQueueFamilyIndex();
}

void SetVkQueueFamilyIndex(MutableTextureState* state, uint32_t queueFamilyIndex) {
    SkASSERT(state->backend() == BackendApi::kVulkan);
    MTSVKPriv::VkState(state).setQueueFamilyIndex(queueFamilyIndex);
}

const VulkanMutableTextureState& GetVulkanState(const MutableTextureState& state) {
    SkASSERT(state.backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state);
}

const VulkanMutableTextureState& GetVulkanState(const MutableTextureState* state) {
    SkASSERT(state->backend() == BackendApi::kVulkan);
    return MTSVKPriv::VkState(state);
}
}  // namespace skgpu::MutableTextureStates
