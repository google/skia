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
                               VkFormat format,
                               const GrVkImage::ImageDesc& desc,
                               const GrVkImageInfo& info,
                               sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                               sk_sp<const GrVkImageView> stencilView)
        : GrAttachment(gpu, dimensions, supportedUsages, desc.fSamples, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
        , fStencilView(std::move(stencilView)) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrVkAttachment> GrVkAttachment::MakeStencil(GrVkGpu* gpu,
                                                  SkISize dimensions,
                                                  int sampleCnt,
                                                  VkFormat format) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = 1;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> imageView = GrVkImageView::Make(
            gpu, info.fImage, format, GrVkImageView::kStencil_Type, 1, GrVkYcbcrConversionInfo());
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkAttachment>(
            new GrVkAttachment(gpu, dimensions, UsageFlags::kStencil, format, imageDesc,
                               info, std::move(mutableState), std::move(imageView)));
}

GrVkAttachment::~GrVkAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fStencilView);
}

size_t GrVkAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= GrVkCaps::GetStencilFormatTotalBitCount(this->imageFormat());
    size *= this->numSamples();
    return static_cast<size_t>(size / 8);
}

void GrVkAttachment::onRelease() {
    this->releaseImage();
    fStencilView.reset();

    GrAttachment::onRelease();
}

void GrVkAttachment::onAbandon() {
    this->releaseImage();
    fStencilView.reset();

    GrAttachment::onAbandon();
}

GrVkGpu* GrVkAttachment::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
