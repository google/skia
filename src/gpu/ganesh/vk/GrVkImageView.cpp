/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "src/gpu/ganesh/vk/GrVkImageView.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

sk_sp<const GrVkImageView> GrVkImageView::Make(GrVkGpu* gpu,
                                               VkImage image,
                                               VkFormat format,
                                               Type viewType,
                                               uint32_t miplevels,
                                               const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo) {
    void* pNext = nullptr;
    VkSamplerYcbcrConversionInfo conversionInfo;
    GrVkSamplerYcbcrConversion* ycbcrConversion = nullptr;

    if (ycbcrInfo.isValid()) {
        SkASSERT(gpu->vkCaps().supportsYcbcrConversion() && format == ycbcrInfo.fFormat);

        ycbcrConversion =
                gpu->resourceProvider().findOrCreateCompatibleSamplerYcbcrConversion(ycbcrInfo);
        if (!ycbcrConversion) {
            return nullptr;
        }

        conversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        conversionInfo.pNext = nullptr;
        conversionInfo.conversion = ycbcrConversion->ycbcrConversion();
        pNext = &conversionInfo;
    }

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
    if (kStencil_Type == viewType) {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateImageView(gpu->device(), &viewInfo, nullptr, &imageView));
    if (err) {
        return nullptr;
    }

    return sk_sp<const GrVkImageView>(new GrVkImageView(gpu, imageView, ycbcrConversion));
}

void GrVkImageView::freeGPUData() const {
    GR_VK_CALL(fGpu->vkInterface(), DestroyImageView(fGpu->device(), fImageView, nullptr));

    if (fYcbcrConversion) {
        fYcbcrConversion->unref();
    }
}

