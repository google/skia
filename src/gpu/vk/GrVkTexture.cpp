/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTexture.h"

#include "GrTexturePriv.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkTextureRenderTarget.h"
#include "GrVkUtil.h"

#include "vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// This method parallels GrTextureProxy::highestFilterMode
static inline GrSamplerState::Filter highest_filter_mode(GrPixelConfig config) {
    if (GrPixelConfigIsSint(config)) {
        // We only ever want to nearest-neighbor sample signed int textures.
        return GrSamplerState::Filter::kNearest;
    }
    return GrSamplerState::Filter::kMipMap;
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkBudgeted budgeted,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , GrVkImage(info, GrBackendObjectOwnership::kOwned)
    , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                mipMapsStatus)
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    this->registerWithCache(budgeted);
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         Wrapped,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus,
                         GrBackendObjectOwnership ownership)
    : GrSurface(gpu, desc)
    , GrVkImage(info, ownership)
    , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                mipMapsStatus)
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
    this->registerWithCacheWrapped();
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         const GrSurfaceDesc& desc,
                         const GrVkImageInfo& info,
                         const GrVkImageView* view,
                         GrMipMapsStatus mipMapsStatus,
                         GrBackendObjectOwnership ownership)
    : GrSurface(gpu, desc)
    , GrVkImage(info, ownership)
    , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                mipMapsStatus)
    , fTextureView(view)
    , fLinearTextureView(nullptr) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == info.fLevelCount));
}

sk_sp<GrVkTexture> GrVkTexture::CreateNewTexture(GrVkGpu* gpu, SkBudgeted budgeted,
                                                 const GrSurfaceDesc& desc,
                                                 const GrVkImage::ImageDesc& imageDesc,
                                                 GrMipMapsStatus mipMapsStatus) {
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

    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, budgeted, desc, info, imageView,
                                              mipMapsStatus));
}

sk_sp<GrVkTexture> GrVkTexture::MakeWrappedTexture(GrVkGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   GrWrapOwnership wrapOwnership,
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

    GrMipMapsStatus mipMapsStatus = info->fLevelCount > 1 ? GrMipMapsStatus::kValid
                                                          : GrMipMapsStatus::kNotAllocated;

    GrBackendObjectOwnership ownership = kBorrow_GrWrapOwnership == wrapOwnership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;
    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, kWrapped, desc, *info, imageView,
                                              mipMapsStatus, ownership));
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

GrBackendTexture GrVkTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo);
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

bool GrVkTexture::reallocForMipmap(GrVkGpu* gpu, uint32_t mipLevels) {
    if (mipLevels == 1) {
        // don't need to do anything for a 1x1 texture
        return false;
    }

    const GrVkResource* oldResource = this->resource();

    // We shouldn't realloc something that doesn't belong to us
    if (fIsBorrowed) {
        return false;
    }

    bool renderTarget = SkToBool(this->asRenderTarget());

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (renderTarget) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = fInfo.fFormat;
    imageDesc.fWidth = this->width();
    imageDesc.fHeight = this->height();
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

    if (renderTarget) {
        GrVkTextureRenderTarget* texRT = static_cast<GrVkTextureRenderTarget*>(this);
        if (!texRT->updateForMipmap(gpu, info)) {
            GrVkImage::DestroyImageInfo(gpu, &info);
            return false;
        }
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
    // SetMaxMipMapLevel stores the max level not the number of levels
    this->texturePriv().setMaxMipMapLevel(mipLevels-1);

    return true;
}
