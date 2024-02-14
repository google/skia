/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanTexture_DEFINED
#define skgpu_graphite_VulkanTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanImageView.h"

#include <utility>

namespace skgpu { class MutableTextureState; }

namespace skgpu::graphite {

class VulkanSharedContext;
class VulkanCommandBuffer;
class VulkanResourceProvider;

class VulkanTexture : public Texture {
public:
    struct CreatedImageInfo {
        VkImage fImage = VK_NULL_HANDLE;
        VulkanAlloc fMemoryAlloc;
        sk_sp<MutableTextureState> fMutableState;
    };

    static bool MakeVkImage(const VulkanSharedContext*,
                            SkISize dimensions,
                            const TextureInfo&,
                            CreatedImageInfo* outInfo);

    static sk_sp<Texture> Make(const VulkanSharedContext*,
                               const VulkanResourceProvider*,
                               SkISize dimensions,
                               const TextureInfo&,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const VulkanSharedContext*,
                                      const VulkanResourceProvider*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_sp<MutableTextureState>,
                                      VkImage,
                                      const VulkanAlloc&);

    ~VulkanTexture() override {}

    VkImage vkImage() const { return fImage; }

    void setImageLayout(VulkanCommandBuffer* buffer,
                        VkImageLayout newLayout,
                        VkAccessFlags dstAccessMask,
                        VkPipelineStageFlags dstStageMask,
                        bool byRegion) const {
        this->setImageLayoutAndQueueIndex(buffer, newLayout, dstAccessMask, dstStageMask, byRegion,
                                          VK_QUEUE_FAMILY_IGNORED);
    }

    void setImageLayoutAndQueueIndex(VulkanCommandBuffer*,
                                     VkImageLayout newLayout,
                                     VkAccessFlags dstAccessMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     uint32_t newQueueFamilyIndex) const;

    // This simply updates our internal tracking of the image layout and does not actually perform
    // any gpu work.
    void updateImageLayout(VkImageLayout);

    VkImageLayout currentLayout() const;
    uint32_t currentQueueFamilyIndex() const;

    const VulkanImageView* getImageView(VulkanImageView::Usage) const;

    // Helpers to use for setting the layout of the VkImage
    static VkPipelineStageFlags LayoutToPipelineSrcStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

    bool supportsInputAttachmentUsage() const {
        return (this->textureInfo().vulkanTextureSpec().fImageUsageFlags &
                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    }

private:
    VulkanTexture(const VulkanSharedContext* sharedContext,
                  SkISize dimensions,
                  const TextureInfo& info,
                  sk_sp<MutableTextureState>,
                  VkImage,
                  const VulkanAlloc&,
                  Ownership,
                  skgpu::Budgeted,
                  sk_sp<VulkanSamplerYcbcrConversion>);

    void freeGpuData() override;

    VkImage fImage;
    VulkanAlloc fMemoryAlloc;
    sk_sp<VulkanSamplerYcbcrConversion> fSamplerYcbcrConversion;

    mutable skia_private::STArray<2, std::unique_ptr<const VulkanImageView>> fImageViews;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanTexture_DEFINED

