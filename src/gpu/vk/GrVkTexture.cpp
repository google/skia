/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTexture.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkUtil.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         GrGpuResource::LifeCycle lifeCycle,
                         const GrVkImage::Resource* imageResource,
                         const GrVkImageView* view)
    : GrSurface(gpu, lifeCycle, desc)
    , GrVkImage(imageResource)
    , INHERITED(gpu, lifeCycle, desc, kSampler2D_GrSLType,
                false) // false because we don't upload MIP data in Vk yet
    , fTextureView(view) {
    this->registerWithCache();
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         GrGpuResource::LifeCycle lifeCycle,
                         const GrVkImage::Resource* imageResource,
                         const GrVkImageView* view,
                         Derived)
    : GrSurface(gpu, lifeCycle, desc)
    , GrVkImage(imageResource)
    , INHERITED(gpu, lifeCycle, desc, kSampler2D_GrSLType,
                false) // false because we don't upload MIP data in Vk yet
    , fTextureView(view) {}


GrVkTexture* GrVkTexture::Create(GrVkGpu* gpu,
                                 const GrSurfaceDesc& desc,
                                 GrGpuResource::LifeCycle lifeCycle,
                                 VkFormat format,
                                 const GrVkImage::Resource* imageResource) {
    VkImage image = imageResource->fImage;
    const GrVkImageView* imageView = GrVkImageView::Create(gpu, image, format,
                                                           GrVkImageView::kColor_Type);
    if (!imageView) {
        return nullptr;
    }

    return new GrVkTexture(gpu, desc, lifeCycle, imageResource, imageView);
}

GrVkTexture* GrVkTexture::CreateNewTexture(GrVkGpu* gpu, const GrSurfaceDesc& desc,
                                           GrGpuResource::LifeCycle lifeCycle,
                                           const GrVkImage::ImageDesc& imageDesc) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    const GrVkImage::Resource* imageResource = GrVkImage::CreateResource(gpu, imageDesc);
    if (!imageResource) {
        return nullptr;
    }

    GrVkTexture* texture = Create(gpu, desc, lifeCycle, imageDesc.fFormat, imageResource);
    // Create() will increment the refCount of the image resource if it succeeds
    imageResource->unref(gpu);

    return texture;
}

GrVkTexture* GrVkTexture::CreateWrappedTexture(GrVkGpu* gpu, const GrSurfaceDesc& desc,
                                               GrGpuResource::LifeCycle lifeCycle,
                                               VkFormat format,
                                               const GrVkTextureInfo* info) {
    SkASSERT(info);
    // Wrapped textures require both image and allocation (because they can be mapped)
    SkASSERT(VK_NULL_HANDLE != info->fImage && VK_NULL_HANDLE != info->fAlloc);

    GrVkImage::Resource::Flags flags = (VK_IMAGE_TILING_LINEAR == info->fImageTiling)
                                     ? Resource::kLinearTiling_Flag : Resource::kNo_Flags;

    const GrVkImage::Resource* imageResource;
    if (kBorrowed_LifeCycle == lifeCycle) {
        imageResource = new GrVkImage::BorrowedResource(info->fImage, info->fAlloc, flags);
    } else {
        imageResource = new GrVkImage::Resource(info->fImage, info->fAlloc, flags);
    }
    if (!imageResource) {
        return nullptr;
    }

    GrVkTexture* texture = Create(gpu, desc, lifeCycle, format, imageResource);
    if (texture) {
        texture->fCurrentLayout = info->fImageLayout;
    }
    // Create() will increment the refCount of the image resource if it succeeds
    imageResource->unref(gpu);

    return texture;
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTextureView);
}

void GrVkTexture::onRelease() {
    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView->unref(this->getVkGpu());
        fTextureView = nullptr;
    }

    if (this->shouldFreeResources()) {
        this->releaseImage(this->getVkGpu());
    } else {
        this->abandonImage();
    }

    INHERITED::onRelease();
}

void GrVkTexture::onAbandon() {
    if (fTextureView) {
        fTextureView->unrefAndAbandon();
        fTextureView = nullptr;
    }

    this->abandonImage();
    INHERITED::onAbandon();
}

GrBackendObject GrVkTexture::getTextureHandle() const {
    // Currently just passing back the pointer to the Resource as the handle
    return (GrBackendObject)&fResource;
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
