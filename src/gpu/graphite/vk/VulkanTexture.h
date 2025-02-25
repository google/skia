/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanTexture_DEFINED
#define skgpu_graphite_VulkanTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/vk/VulkanImageView.h"

#include <utility>

namespace skgpu { class MutableTextureState; }

namespace skgpu::graphite {

class Sampler;
class VulkanSharedContext;
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanFramebuffer;
class VulkanResourceProvider;
struct RenderPassDesc;

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
                               SkISize dimensions,
                               const TextureInfo&,
                               sk_sp<VulkanYcbcrConversion>);

    static sk_sp<Texture> MakeWrapped(const VulkanSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_sp<MutableTextureState>,
                                      VkImage,
                                      const VulkanAlloc&,
                                      sk_sp<VulkanYcbcrConversion>);

    ~VulkanTexture() override;

    const VulkanTextureInfo& vulkanTextureInfo() const {
        return TextureInfoPriv::Get<VulkanTextureInfo>(this->textureInfo());
    }
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

    bool supportsInputAttachmentUsage() const;

    sk_sp<VulkanDescriptorSet> getCachedSingleTextureDescriptorSet(const Sampler*) const;
    void addCachedSingleTextureDescriptorSet(sk_sp<VulkanDescriptorSet>,
                                            sk_sp<const Sampler>) const;

    sk_sp<VulkanFramebuffer> getCachedFramebuffer(const RenderPassDesc& renderPassDesc,
                                                  const VulkanTexture* msaaTexture,
                                                  const VulkanTexture* depthStencilTexture) const;
    void addCachedFramebuffer(sk_sp<VulkanFramebuffer>);

private:
    VulkanTexture(const VulkanSharedContext* sharedContext,
                  SkISize dimensions,
                  const TextureInfo& info,
                  sk_sp<MutableTextureState>,
                  VkImage,
                  const VulkanAlloc&,
                  Ownership,
                  sk_sp<VulkanYcbcrConversion>);

    void freeGpuData() override;

    size_t onUpdateGpuMemorySize() override;

    VkImage fImage;
    VulkanAlloc fMemoryAlloc;
    sk_sp<VulkanYcbcrConversion> fYcbcrConversion;

    mutable skia_private::STArray<2, std::unique_ptr<const VulkanImageView>> fImageViews;

    using CachedTextureDescSet = std::pair<sk_sp<const Sampler>, sk_sp<VulkanDescriptorSet>>;
    mutable skia_private::STArray<3, CachedTextureDescSet> fCachedSingleTextureDescSets;

    skia_private::STArray<3, sk_sp<VulkanFramebuffer>> fCachedFramebuffers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanTexture_DEFINED
