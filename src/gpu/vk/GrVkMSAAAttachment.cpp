/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkMSAAAttachment.h"

#include "src/gpu/GrBackendSurfaceMutableStateImpl.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"

sk_sp<GrVkMSAAAttachment> GrVkMSAAAttachment::Make(GrVkGpu* gpu,
                                                   SkISize dimensions,
                                                   const GrVkImage::ImageDesc& imageDesc) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> imageView =
            GrVkImageView::Make(gpu, info.fImage, info.fFormat, GrVkImageView::kColor_Type,
                                info.fLevelCount, info.fYcbcrConversionInfo);
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkMSAAAttachment>(new GrVkMSAAAttachment(
            gpu, dimensions, imageDesc.fSamples, info, std::move(mutableState), imageView));
}

GrVkMSAAAttachment::GrVkMSAAAttachment(GrVkGpu* gpu,
                                       SkISize dimensions,
                                       int sampleCnt,
                                       const GrVkImageInfo& info,
                                       sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                                       sk_sp<const GrVkImageView> view)
        : GrMSAAAttachment(gpu, dimensions, sampleCnt, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
        , fView(std::move(view)) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrVkMSAAAttachment::~GrVkMSAAAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fView);
}

size_t GrVkMSAAAttachment::onGpuMemorySize() const {
    int numColorSamples = this->sampleCnt();
    SkASSERT(numColorSamples > 1);
    const GrCaps& caps = *this->getGpu()->caps();
    return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(), numColorSamples,
                                  GrMipmapped::kNo);
}

void GrVkMSAAAttachment::onRelease() {
    this->releaseImage();
    fView.reset();

    GrMSAAAttachment::onRelease();
}

void GrVkMSAAAttachment::onAbandon() {
    this->releaseImage();
    fView.reset();

    GrMSAAAttachment::onAbandon();
}
