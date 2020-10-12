/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkAttachment.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkAttachment::GrVkAttachment(GrVkGpu* gpu,
                               SkISize dimensions,
                               UsageFlags supportedUsages,
                               const GrVkImageInfo& info,
                               sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                               sk_sp<const GrVkImageView> view)
        : GrAttachment(gpu, dimensions, supportedUsages, info.fSampleCount, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
        , fView(std::move(view)) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeStencil(GrVkGpu* gpu,
                                                  SkISize dimensions,
                                                  int sampleCnt,
                                                  VkFormat format) {
    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkAttachment::Make(gpu, dimensions, UsageFlags::kStencil, sampleCnt, format,
                                vkUsageFlags, GrProtected::kNo);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeMSAA(GrVkGpu* gpu,
                                               SkISize dimensions,
                                               int numSamples,
                                               VkFormat format,
                                               GrProtected isProtected) {
    SkASSERT(numSamples > 1);

    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkAttachment::Make(gpu, dimensions, UsageFlags::kMSAA, numSamples, format,
                                vkUsageFlags, isProtected);
}

sk_sp<GrVkAttachment> GrVkAttachment::Make(GrVkGpu* gpu,
                                           SkISize dimensions,
                                           UsageFlags attachmentUsages,
                                           int sampleCnt,
                                           VkFormat format,
                                           VkImageUsageFlags vkUsageFlags,
                                           GrProtected isProtected) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = 1;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = vkUsageFlags;
    imageDesc.fIsProtected = isProtected;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    GrVkImageView::Type viewType;
    if (attachmentUsages & UsageFlags::kStencil) {
        // If we have stencil usage than we should have any other usages
        SkASSERT(attachmentUsages == UsageFlags::kStencil);
        viewType = GrVkImageView::kStencil_Type;
    } else {
        viewType = GrVkImageView::kColor_Type;
    }

    sk_sp<const GrVkImageView> imageView = GrVkImageView::Make(
            gpu, info.fImage, format, viewType, info.fLevelCount, info.fYcbcrConversionInfo);
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkAttachment>(new GrVkAttachment(gpu, dimensions, attachmentUsages, info,
                                                    std::move(mutableState), std::move(imageView)));
}

GrVkAttachment::~GrVkAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fView);
}

void GrVkAttachment::onRelease() {
    this->releaseImage();
    fView.reset();

    GrAttachment::onRelease();
}

void GrVkAttachment::onAbandon() {
    this->releaseImage();
    fView.reset();

    GrAttachment::onAbandon();
}

GrVkGpu* GrVkAttachment::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
