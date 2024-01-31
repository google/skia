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
#include "src/gpu/MutableTextureStatePriv.h"
#include "src/gpu/vk/VulkanMutableTextureStatePriv.h"

namespace skgpu::MutableTextureStates {

class VulkanMutableTextureState : public MutableTextureStateData {
public:
    VulkanMutableTextureState(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fLayout(layout)
            , fQueueFamilyIndex(queueFamilyIndex) {}

#if defined(SK_DEBUG)
    BackendApi type() const override { return BackendApi::kVulkan; }
#endif

    void copyTo(AnyStateData& formatData) const override {
        formatData.emplace<VulkanMutableTextureState>(fLayout, fQueueFamilyIndex);
    }

    VkImageLayout fLayout;
    uint32_t      fQueueFamilyIndex;
};

MutableTextureState MakeVulkan(VkImageLayout layout, uint32_t queueFamilyIndex) {
    return MutableTextureStatePriv::MakeMutableTextureState(
            BackendApi::kVulkan,
            VulkanMutableTextureState(layout, queueFamilyIndex));
}

static const VulkanMutableTextureState* get_and_cast_data(const MutableTextureState& mts) {
    auto data = skgpu::MutableTextureStatePriv::GetStateData(mts);
    SkASSERT(!data || data->type() == BackendApi::kVulkan);
    return static_cast<const VulkanMutableTextureState*>(data);
}

static const VulkanMutableTextureState* get_and_cast_data(const MutableTextureState* mts) {
    auto data = skgpu::MutableTextureStatePriv::GetStateData(mts);
    SkASSERT(!data || data->type() == BackendApi::kVulkan);
    return static_cast<const VulkanMutableTextureState*>(data);
}

static VulkanMutableTextureState* get_and_cast_data(MutableTextureState* mts) {
    auto data = skgpu::MutableTextureStatePriv::GetStateData(mts);
    SkASSERT(!data || data->type() == BackendApi::kVulkan);
    return static_cast<VulkanMutableTextureState*>(data);
}

VkImageLayout GetVkImageLayout(const MutableTextureState& state) {
    SkASSERT(state.backend() == BackendApi::kVulkan);
    return get_and_cast_data(state)->fLayout;
}

VkImageLayout GetVkImageLayout(const MutableTextureState* state) {
    SkASSERT(state);
    SkASSERT(state->backend() == BackendApi::kVulkan);
    return get_and_cast_data(state)->fLayout;
}

void SetVkImageLayout(MutableTextureState* state, VkImageLayout layout) {
    SkASSERT(state->backend() == BackendApi::kVulkan);
    get_and_cast_data(state)->fLayout = layout;
}

uint32_t GetVkQueueFamilyIndex(const MutableTextureState& state) {
    SkASSERT(state.backend() == BackendApi::kVulkan);
    return get_and_cast_data(state)->fQueueFamilyIndex;
}

uint32_t GetVkQueueFamilyIndex(const MutableTextureState* state) {
    SkASSERT(state);
    SkASSERT(state->backend() == BackendApi::kVulkan);
    return get_and_cast_data(state)->fQueueFamilyIndex;
}

void SetVkQueueFamilyIndex(MutableTextureState* state, uint32_t queueFamilyIndex) {
    SkASSERT(state->backend() == BackendApi::kVulkan);
    get_and_cast_data(state)->fQueueFamilyIndex = queueFamilyIndex;
}

}  // namespace skgpu::MutableTextureStates
