/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkImageView.h"
#include "GrVkGpu.h"
#include "GrVkSamplerYcbcrConversion.h"
#include "GrVkUtil.h"

const GrVkImageView* GrVkImageView::Create(GrVkGpu* gpu, VkImage image, VkFormat format,
                                           Type viewType, uint32_t miplevels,
                                           const GrVkYcbcrConversionInfo& ycbcrInfo) {

    void* pNext = nullptr;
    VkSamplerYcbcrConversionInfo conversionInfo;
    GrVkSamplerYcbcrConversion* ycbcrConversion = nullptr;

    if (ycbcrInfo.isValid()) {
        SkASSERT(gpu->vkCaps().supportsYcbcrConversion() && format == VK_FORMAT_UNDEFINED);

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

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateImageView(gpu->device(), &viewInfo,
                                                                  nullptr, &imageView));
    if (err) {
        return nullptr;
    }

    return new GrVkImageView(imageView, ycbcrConversion);
}

void GrVkImageView::freeGPUData(GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyImageView(gpu->device(), fImageView, nullptr));

    if (fYcbcrConversion) {
        fYcbcrConversion->unref(gpu);
    }
}

void GrVkImageView::abandonGPUData() const {
    if (fYcbcrConversion) {
        fYcbcrConversion->unrefAndAbandon();
    }
}

