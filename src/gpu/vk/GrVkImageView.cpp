/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkImageView.h"
#include "GrVkGpu.h"
#include "GrVkUtil.h"

const GrVkImageView* GrVkImageView::Create(const GrVkGpu* gpu, VkImage image, VkFormat format,
                                           Type viewType, uint32_t miplevels) {
    VkImageView imageView;

    // Create the VkImageView
    VkImageViewCreateInfo viewInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,               // sType
        NULL,                                                   // pNext
        0,                                                      // flags
        image,                                                  // image
        VK_IMAGE_VIEW_TYPE_2D,                                  // viewType
        format,                                                 // format
        { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
          VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },     // components
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, miplevels, 0, 1 },      // subresourceRange
    };
    if (kStencil_Type == viewType) {
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_ZERO;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_ZERO;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_ZERO;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_ZERO;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateImageView(gpu->device(), &viewInfo,
                                                                  nullptr, &imageView));
    if (err) {
        return nullptr;
    }

    return new GrVkImageView(imageView);
}

void GrVkImageView::freeGPUData(const GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyImageView(gpu->device(), fImageView, nullptr));
}
