/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/vk/VulkanGraphiteTypesPriv.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <cstdint>

namespace skgpu::graphite {

class VulkanTextureInfoData final : public TextureInfoData {
public:
    VulkanTextureInfoData(VulkanTextureSpec v) : fVkSpec(v) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kVulkan; }
#endif

    VulkanTextureSpec spec() const { return fVkSpec; }

private:
    VulkanTextureSpec fVkSpec;

    size_t bytesPerPixel() const override { return VkFormatBytesPerBlock(fVkSpec.fFormat); }

    SkTextureCompressionType compressionType() const override {
        return VkFormatToCompressionType(fVkSpec.fFormat);
    }

    SkString toString() const override {
        return SkStringPrintf("Vulkan(%s,", fVkSpec.toString().c_str());
    }

    SkString toRPAttachmentString(uint32_t sampleCount) const override {
        return SkStringPrintf(
                "Vulkan(f=%u,s=%u)", static_cast<unsigned int>(fVkSpec.fFormat), sampleCount);
    }

    void copyTo(AnyTextureInfoData& dstData) const override {
        // Don't assert that dstData has a Vulkan type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<VulkanTextureInfoData>(fVkSpec);
    }

    bool equal(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kVulkan);
        if (auto otherVk = static_cast<const VulkanTextureInfoData*>(that)) {
            return fVkSpec == otherVk->fVkSpec;
        }
        return false;
    }

    bool isCompatible(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kVulkan);
        if (auto otherVk = static_cast<const VulkanTextureInfoData*>(that)) {
            return fVkSpec.isCompatible(otherVk->fVkSpec);
        }
        return false;
    }
};

static const VulkanTextureInfoData* get_and_cast_data(const TextureInfo& info) {
    auto data = TextureInfoPriv::GetData(info);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<const VulkanTextureInfoData*>(data);
}

namespace TextureInfos {
TextureInfo MakeVulkan(const VulkanTextureInfo& vkInfo) {
    skgpu::Protected p = Protected::kNo;
    if (vkInfo.fFlags & VK_IMAGE_CREATE_PROTECTED_BIT) {
        p = Protected::kYes;
    }
    return TextureInfoPriv::Make(skgpu::BackendApi::kVulkan,
                                 vkInfo.fSampleCount,
                                 vkInfo.fMipmapped,
                                 p,
                                 VulkanTextureInfoData(vkInfo));
}

bool GetVulkanTextureInfo(const TextureInfo& info, VulkanTextureInfo* out) {
    if (!info.isValid() || info.backend() != skgpu::BackendApi::kVulkan) {
        return false;
    }
    SkASSERT(out);
    const VulkanTextureInfoData* vkData = get_and_cast_data(info);
    SkASSERT(vkData);
    *out = VulkanTextureSpecToTextureInfo(vkData->spec(), info.numSamples(), info.mipmapped());
    return true;
}

// This cannot return a const reference or we get a warning about returning
// a reference to a temporary local variable.
VulkanTextureSpec GetVulkanTextureSpec(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kVulkan);
    const VulkanTextureInfoData* vkData = get_and_cast_data(info);
    SkASSERT(vkData);
    return vkData->spec();
}

VkFormat GetVkFormat(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kVulkan);
    const VulkanTextureInfoData* vkData = get_and_cast_data(info);
    SkASSERT(vkData);
    return vkData->spec().fFormat;
}

VkImageUsageFlags GetVkUsageFlags(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kVulkan);
    const VulkanTextureInfoData* vkData = get_and_cast_data(info);
    SkASSERT(vkData);
    return vkData->spec().fImageUsageFlags;
}

VulkanYcbcrConversionInfo GetVulkanYcbcrConversionInfo(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kVulkan);
    const VulkanTextureInfoData* vkData = get_and_cast_data(info);
    SkASSERT(vkData);
    return vkData->spec().fYcbcrConversionInfo;
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
