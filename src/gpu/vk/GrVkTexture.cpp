/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTexture.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrTexturePriv.h"
#include "GrVkUtil.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkBudgeted budgeted,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view)
    : GrSurface(gpu, desc)
    , GrVkImage(info, GrVkImage::kNot_Wrapped)
    , INHERITED(gpu, desc, kSampler2D_GrSLType, desc.fIsMipMapped) 
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
    this->registerWithCache(budgeted);
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         Wrapped,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view,
                         GrVkImage::Wrapped wrapped)
    : GrSurface(gpu, desc)
    , GrVkImage(info, wrapped)
    , INHERITED(gpu, desc, kSampler2D_GrSLType, desc.fIsMipMapped)
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
    this->registerWithCacheWrapped();
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view,
                         GrVkImage::Wrapped wrapped)
    : GrSurface(gpu, desc)
    , GrVkImage(info, wrapped)
    , INHERITED(gpu, desc, kSampler2D_GrSLType, desc.fIsMipMapped)
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
}

GrVkTexture* GrVkTexture::CreateNewTexture(GrVkGpu* gpu, SkBudgeted budgeted,
                                           const GrSurfaceDesc& desc,
                                           const GrVkImage::ImageDesc& imageDesc) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    const GrVkImageView* imageView = GrVkImageView::Create(gpu, info.fImage, info.fFormat,
                                                           GrVkImageView::kColor_Type,
                                                           info.fLevelCount);
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    return new GrVkTexture(gpu, budgeted, desc, info, imageView);
}

GrVkTexture* GrVkTexture::CreateWrappedTexture(GrVkGpu* gpu,
                                               const GrSurfaceDesc& desc,
                                               GrWrapOwnership ownership,
                                               const GrVkImageInfo* info) {
    SkASSERT(info);
    // Wrapped textures require both image and allocation (because they can be mapped)
    SkASSERT(VK_NULL_HANDLE != info->fImage && VK_NULL_HANDLE != info->fAlloc.fMemory);

    const GrVkImageView* imageView = GrVkImageView::Create(gpu, info->fImage, info->fFormat,
                                                           GrVkImageView::kColor_Type,
                                                           info->fLevelCount);
    if (!imageView) {
        return nullptr;
    }

    GrVkImage::Wrapped wrapped = kBorrow_GrWrapOwnership == ownership ? GrVkImage::kBorrowed_Wrapped
                                                                      : GrVkImage::kAdopted_Wrapped;

    return new GrVkTexture(gpu, kWrapped, desc, *info, imageView, wrapped);
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTextureView);
    SkASSERT(!fLinearTextureView);
}

void GrVkTexture::onRelease() {
    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView->unref(this->getVkGpu());
        fTextureView = nullptr;
    }

    if (fLinearTextureView) {
        fLinearTextureView->unref(this->getVkGpu());
        fLinearTextureView = nullptr;
    }

    this->releaseImage(this->getVkGpu());

    INHERITED::onRelease();
}

void GrVkTexture::onAbandon() {
    if (fTextureView) {
        fTextureView->unrefAndAbandon();
        fTextureView = nullptr;
    }

    if (fLinearTextureView) {
        fLinearTextureView->unrefAndAbandon();
        fLinearTextureView = nullptr;
    }

    this->abandonImage();
    INHERITED::onAbandon();
}

GrBackendObject GrVkTexture::getTextureHandle() const {
    return (GrBackendObject)&fInfo;
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

const GrVkImageView* GrVkTexture::textureView(bool allowSRGB) {
    VkFormat linearFormat;
    if (allowSRGB || !GrVkFormatIsSRGB(fInfo.fFormat, &linearFormat)) {
        return fTextureView;
    }

    if (!fLinearTextureView) {
        fLinearTextureView = GrVkImageView::Create(this->getVkGpu(), fInfo.fImage,
                                                   linearFormat, GrVkImageView::kColor_Type,
                                                   fInfo.fLevelCount);
        SkASSERT(fLinearTextureView);
    }

    return fLinearTextureView;
}

bool GrVkTexture::reallocForMipmap(const GrVkGpu* gpu, uint32_t mipLevels) {
    if (mipLevels == 1) {
        // don't need to do anything for a 1x1 texture
        return false;
    }

    const GrVkResource* oldResource = this->resource();

    // We shouldn't realloc something that doesn't belong to us
    if (fIsBorrowed) {
        return false;
    }

    // Does this even make sense for rendertargets?
    bool renderTarget = SkToBool(fDesc.fFlags & kRenderTarget_GrSurfaceFlag);

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (renderTarget) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = fInfo.fFormat;
    imageDesc.fWidth = fDesc.fWidth;
    imageDesc.fHeight = fDesc.fHeight;
    imageDesc.fLevels = mipLevels;
    imageDesc.fSamples = 1;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = usageFlags;
    imageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return false;
    }

    // have to create a new image view for new resource
    const GrVkImageView* oldView = fTextureView;
    VkImage image = info.fImage;
    const GrVkImageView* textureView = GrVkImageView::Create(gpu, image, info.fFormat,
                                                             GrVkImageView::kColor_Type, mipLevels);
    if (!textureView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return false;
    }

    oldResource->unref(gpu);
    oldView->unref(gpu);
    if (fLinearTextureView) {
        fLinearTextureView->unref(gpu);
        fLinearTextureView = nullptr;
    }

    this->setNewResource(info.fImage, info.fAlloc, info.fImageTiling);
    fTextureView = textureView;
    fInfo = info;
    this->texturePriv().setMaxMipMapLevel(mipLevels);

    return true;
}
