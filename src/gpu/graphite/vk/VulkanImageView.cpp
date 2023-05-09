/*
* Copyright 2023 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanImageView.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<const VulkanImageView> VulkanImageView::Make(VulkanSharedContext* sharedContext,
                                                   VkImage image,
                                                   VkFormat format,
                                                   Type viewType,
                                                   uint32_t miplevels) {
    void* pNext = nullptr;
    // TODO: add SamplerYcbcrConversion setup

    VkImageView imageView;
    // Create the VkImageView
    VkImageViewCreateInfo viewInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,               // sType
        pNext,                                                  // pNext
        0,                                                      // flags
        image,                                                  // image
        VK_IMAGE_VIEW_TYPE_2D,                                  // viewType
        format,                                                 // format
        { VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY },                      // components
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, miplevels, 0, 1 },      // subresourceRange
    };
    if (Type::kStencil == viewType) {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkResult result;
    VULKAN_CALL_RESULT(sharedContext->interface(), result,
                       CreateImageView(sharedContext->device(), &viewInfo, nullptr, &imageView));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return sk_sp<const VulkanImageView>(new VulkanImageView(sharedContext, imageView));
}

VulkanImageView::VulkanImageView(const VulkanSharedContext* sharedContext, VkImageView imageView)
    : Resource(sharedContext, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0)
    , fImageView(imageView) {
}

void VulkanImageView::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VULKAN_CALL(sharedContext->interface(),
                DestroyImageView(sharedContext->device(), fImageView, nullptr));

    // TODO: unref SamplerYcbcrConversion
}

}  // namespace skgpu::graphite
