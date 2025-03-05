/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphiteTypes_DEFINED
#define skgpu_graphite_VulkanGraphiteTypes_DEFINED

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/vk/VulkanTypes.h"

class SkStream;
class SkWStream;

namespace skgpu::graphite {

class SK_API VulkanTextureInfo final : public TextureInfo::Data {
public:
    // VkImageCreateInfo properties
    // Currently the only supported flag is VK_IMAGE_CREATE_PROTECTED_BIT. Any other flag will not
    // be accepted
    VkImageCreateFlags fFlags = 0;
    VkFormat           fFormat = VK_FORMAT_UNDEFINED;
    VkImageTiling      fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags  fImageUsageFlags = 0;
    VkSharingMode      fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Properties related to the image view and sampling. These are less inherent properties of the
    // VkImage but describe how the VkImage should be used within Skia.

    // What aspect to use for the VkImageView. The normal, default is VK_IMAGE_ASPECT_COLOR_BIT.
    // However, if the VkImage is a Ycbcr format, the client can pass a specific plan here to have
    // Skia directly sample a plane. In that case the client should also pass in a VkFormat that is
    // compatible with the plane as described by the Vulkan spec.
    VkImageAspectFlags        fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VulkanYcbcrConversionInfo fYcbcrConversionInfo;

    VulkanTextureInfo() = default;
    VulkanTextureInfo(uint32_t sampleCount,
                      Mipmapped mipmapped,
                      VkImageCreateFlags flags,
                      VkFormat format,
                      VkImageTiling imageTiling,
                      VkImageUsageFlags imageUsageFlags,
                      VkSharingMode sharingMode,
                      VkImageAspectFlags aspectMask,
                      VulkanYcbcrConversionInfo ycbcrConversionInfo)
            : Data(sampleCount, mipmapped)
            , fFlags(flags)
            , fFormat(format)
            , fImageTiling(imageTiling)
            , fImageUsageFlags(imageUsageFlags)
            , fSharingMode(sharingMode)
            , fAspectMask(aspectMask)
            , fYcbcrConversionInfo(ycbcrConversionInfo) {}

private:
    friend class TextureInfo;
    friend class TextureInfoPriv;

    // Non-virtual template API for TextureInfo::Data accessed directly when backend type is known.
    static constexpr skgpu::BackendApi kBackend = skgpu::BackendApi::kVulkan;

    Protected isProtected() const {
        return fFlags & VK_IMAGE_CREATE_PROTECTED_BIT ? Protected::kYes : Protected::kNo;
    }
    TextureFormat viewFormat() const;

    bool serialize(SkWStream*) const;
    bool deserialize(SkStream*);

    // Virtual API when the specific backend type is not available.
    SkString toBackendString() const override;

    void copyTo(TextureInfo::AnyTextureInfoData& dstData) const override {
        dstData.emplace<VulkanTextureInfo>(*this);
    }
    bool isCompatible(const TextureInfo& that, bool requireExact) const override;
};

namespace TextureInfos {
SK_API TextureInfo MakeVulkan(const VulkanTextureInfo&);

SK_API bool GetVulkanTextureInfo(const TextureInfo&, VulkanTextureInfo*);
}  // namespace TextureInfos

namespace BackendTextures {
SK_API BackendTexture MakeVulkan(SkISize dimensions,
                                 const VulkanTextureInfo&,
                                 VkImageLayout,
                                 uint32_t queueFamilyIndex,
                                 VkImage,
                                 VulkanAlloc);
}  // namespace BackendTextures

namespace BackendSemaphores {
SK_API BackendSemaphore MakeVulkan(VkSemaphore);

SK_API VkSemaphore GetVkSemaphore(const BackendSemaphore&);
}  // namespace BackendSemaphores

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_VulkanGraphiteTypes_DEFINED
