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
#include "src/gpu/graphite/Texture.h"

#include <utility>

namespace skgpu { class MutableTextureStateRef; }

namespace skgpu::graphite {

class VulkanSharedContext;
class VulkanCommandBuffer;

class VulkanTexture : public Texture {
public:
    struct CreatedImageInfo {
        VkImage fImage = VK_NULL_HANDLE;
        VulkanAlloc fMemoryAlloc;
        sk_sp<MutableTextureStateRef> fMutableState;
    };

    static bool MakeVkImage(const VulkanSharedContext*,
                            SkISize dimensions,
                            const TextureInfo&,
                            CreatedImageInfo* outInfo);

    static sk_sp<Texture> Make(const VulkanSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const VulkanSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_sp<MutableTextureStateRef>,
                                      VkImage,
                                      const VulkanAlloc&);

    ~VulkanTexture() override {}

    VkImage vkImage() const { return fImage; }

   void setImageLayout(VulkanCommandBuffer* buffer,
                       VkImageLayout newLayout,
                       VkAccessFlags dstAccessMask,
                       VkPipelineStageFlags dstStageMask,
                       bool byRegion) {
        this->setImageLayoutAndQueueIndex(buffer, newLayout, dstAccessMask, dstStageMask, byRegion,
                                          VK_QUEUE_FAMILY_IGNORED);
    }

    void setImageLayoutAndQueueIndex(VulkanCommandBuffer*,
                                     VkImageLayout newLayout,
                                     VkAccessFlags dstAccessMask,
                                     VkPipelineStageFlags dstStageMask,
                                     bool byRegion,
                                     uint32_t newQueueFamilyIndex);

    VkImageLayout currentLayout() const;
    uint32_t currentQueueFamilyIndex() const;

    // Helpers to use for setting the layout of the VkImage
    static VkPipelineStageFlags LayoutToPipelineSrcStageFlags(const VkImageLayout layout);
    static VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);

private:
    VulkanTexture(const VulkanSharedContext* sharedContext,
                  SkISize dimensions,
                  const TextureInfo& info,
                  sk_sp<MutableTextureStateRef>,
                  VkImage,
                  const VulkanAlloc&,
                  Ownership,
                  skgpu::Budgeted);

    void freeGpuData() override;

    VkImage fImage;
    VulkanAlloc fMemoryAlloc;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanTexture_DEFINED

