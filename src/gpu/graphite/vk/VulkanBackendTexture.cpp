/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

#include <cstdint>

namespace skgpu::graphite {

class VulkanBackendTextureData final : public BackendTextureData {
public:
    VulkanBackendTextureData(VulkanAlloc alloc, sk_sp<skgpu::MutableTextureState> mts, VkImage vImg)
            : fMemoryAlloc(alloc), fMutableState(mts), fVkImage(vImg) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kVulkan; }
#endif

    VkImage image() const { return fVkImage; }
    VulkanAlloc memoryAllocator() const { return fMemoryAlloc; }
    sk_sp<skgpu::MutableTextureState> mutableState() const { return fMutableState; }

private:
    VulkanAlloc fMemoryAlloc;
    sk_sp<skgpu::MutableTextureState> fMutableState;
    VkImage fVkImage;

    void copyTo(AnyBackendTextureData& dstData) const override {
        // Don't assert that dstData has a Vulkan type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<VulkanBackendTextureData>(fMemoryAlloc, fMutableState, fVkImage);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kVulkan);
        if (auto otherVk = static_cast<const VulkanBackendTextureData*>(that)) {
            // We ignore the other two fields for the purpose of comparison.
            return fVkImage == otherVk->fVkImage;
        }
        return false;
    }
};

static const VulkanBackendTextureData* get_and_cast_data(const BackendTexture& tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<const VulkanBackendTextureData*>(data);
}

static VulkanBackendTextureData* get_and_cast_data(BackendTexture* tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<VulkanBackendTextureData*>(data);
}

namespace BackendTextures {
BackendTexture MakeVulkan(SkISize dimensions,
                          const VulkanTextureInfo& info,
                          VkImageLayout layout,
                          uint32_t queueFamilyIndex,
                          VkImage image,
                          VulkanAlloc vulkanMemoryAllocation) {
    return BackendTexturePriv::Make(
            dimensions,
            TextureInfos::MakeVulkan(info),
            VulkanBackendTextureData(
                    vulkanMemoryAllocation,
                    sk_make_sp<skgpu::MutableTextureState>(
                            skgpu::MutableTextureStates::MakeVulkan(layout, queueFamilyIndex)),
                    image));
}

VkImage GetVkImage(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return VK_NULL_HANDLE;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->image();
}

VkImageLayout GetVkImageLayout(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return skgpu::MutableTextureStates::GetVkImageLayout(vkData->mutableState().get());
}

uint32_t GetVkQueueFamilyIndex(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return 0;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return skgpu::MutableTextureStates::GetVkQueueFamilyIndex(vkData->mutableState().get());
}

VulkanAlloc GetMemoryAlloc(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return {};
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->memoryAllocator();
}

sk_sp<skgpu::MutableTextureState> GetMutableState(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return {};
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->mutableState();
}

void SetMutableState(BackendTexture* tex, const skgpu::MutableTextureState& newState) {
    SkASSERT(tex);
    if (!tex->isValid() || tex->backend() != skgpu::BackendApi::kVulkan) {
        return;
    }
    VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    vkData->mutableState()->set(newState);
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite
