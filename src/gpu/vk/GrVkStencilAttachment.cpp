/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkStencilAttachment.h"
#include "GrVkGpu.h"
#include "GrVkImage.h"
#include "GrVkImageView.h"
#include "GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkStencilAttachment::GrVkStencilAttachment(GrVkGpu* gpu,
                                             GrGpuResource::LifeCycle lifeCycle,
                                             const Format& format,
                                             const GrVkImage::ImageDesc& desc,
                                             const GrVkImage::Resource* imageResource,
                                             const GrVkImageView* stencilView)
    : GrStencilAttachment(gpu, lifeCycle, desc.fWidth, desc.fHeight,
                          format.fStencilBits, desc.fSamples)
    , GrVkImage(imageResource)
    , fFormat(format)
    , fStencilView(stencilView) {
    this->registerWithCache();
    stencilView->ref();
}

GrVkStencilAttachment* GrVkStencilAttachment::Create(GrVkGpu* gpu,
                                                     GrGpuResource::LifeCycle lifeCycle,
                                                     int width,
                                                     int height,
                                                     int sampleCnt,
                                                     const Format& format) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format.fInternalFormat;
    imageDesc.fWidth = width;
    imageDesc.fHeight = height;
    imageDesc.fLevels = 1;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const GrVkImage::Resource* imageResource = GrVkImage::CreateResource(gpu, imageDesc);
    if (!imageResource) {
        return nullptr;
    }

    const GrVkImageView* imageView = GrVkImageView::Create(gpu, imageResource->fImage,
                                                           format.fInternalFormat,
                                                           GrVkImageView::kStencil_Type);
    if (!imageView) {
        imageResource->unref(gpu);
        return nullptr;
    }

    GrVkStencilAttachment* stencil = new GrVkStencilAttachment(gpu, lifeCycle, format, imageDesc,
                                                               imageResource, imageView);
    imageResource->unref(gpu);
    imageView->unref(gpu);

    return stencil;
}

GrVkStencilAttachment::~GrVkStencilAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fStencilView);
}

size_t GrVkStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= fFormat.fTotalBits;
    size *= SkTMax(1,this->numSamples());
    return static_cast<size_t>(size / 8);
}

void GrVkStencilAttachment::onRelease() {
    GrVkGpu* gpu = this->getVkGpu();

    this->releaseImage(gpu);

    fStencilView->unref(gpu);
    fStencilView = nullptr;
    GrStencilAttachment::onRelease();
}

void GrVkStencilAttachment::onAbandon() {
    this->abandonImage();
    fStencilView->unrefAndAbandon();
    fStencilView = nullptr;
    GrStencilAttachment::onAbandon();
}

GrVkGpu* GrVkStencilAttachment::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
