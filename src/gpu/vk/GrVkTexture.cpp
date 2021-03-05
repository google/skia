/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkTexture.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkTextureRenderTarget.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkBudgeted budgeted,
                         SkISize dimensions,
                         const GrVkImageInfo& info,
                         sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                         sk_sp<const GrVkImageView> view,
                         GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), GrBackendObjectOwnership::kOwned)
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipmapStatus)
        , fTextureView(std::move(view))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
    // We don't support creating external GrVkTextures
    SkASSERT(!info.fYcbcrConversionInfo.isValid() || !info.fYcbcrConversionInfo.fExternalFormat);
    SkASSERT(SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT));
    this->registerWithCache(budgeted);
    if (GrVkFormatIsCompressed(info.fFormat)) {
        this->setReadOnly();
    }
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu, SkISize dimensions, const GrVkImageInfo& info,
                         sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                         sk_sp<const GrVkImageView> view,
                         GrMipmapStatus mipmapStatus, GrBackendObjectOwnership ownership,
                         GrWrapCacheable cacheable, GrIOType ioType, bool isExternal)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), ownership)
        , INHERITED(gpu, dimensions, info.fProtected,
                    isExternal ? GrTextureType::kExternal : GrTextureType::k2D, mipmapStatus)
        , fTextureView(std::move(view))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
    SkASSERT(SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkISize dimensions,
                         const GrVkImageInfo& info,
                         sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                         sk_sp<const GrVkImageView> view,
                         GrMipmapStatus mipmapStatus,
                         GrBackendObjectOwnership ownership)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, std::move(mutableState), ownership)
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipmapStatus)
        , fTextureView(std::move(view))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
    // Since this ctor is only called from GrVkTextureRenderTarget, we can't have a ycbcr conversion
    // since we don't support that on render targets.
    SkASSERT(!info.fYcbcrConversionInfo.isValid());
    SkASSERT(SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT));
}

sk_sp<GrVkTexture> GrVkTexture::MakeNewTexture(GrVkGpu* gpu, SkBudgeted budgeted,
                                               SkISize dimensions,
                                               const GrVkImage::ImageDesc& imageDesc,
                                               GrMipmapStatus mipmapStatus) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> imageView = GrVkImageView::Make(
            gpu, info.fImage, info.fFormat, GrVkImageView::kColor_Type, info.fLevelCount,
            info.fYcbcrConversionInfo);
    if (!imageView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, budgeted, dimensions, info,
                                              std::move(mutableState), std::move(imageView),
                                              mipmapStatus));
}

sk_sp<GrVkTexture> GrVkTexture::MakeWrappedTexture(
        GrVkGpu* gpu, SkISize dimensions, GrWrapOwnership wrapOwnership, GrWrapCacheable cacheable,
        GrIOType ioType, const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    sk_sp<const GrVkImageView> imageView = GrVkImageView::Make(
            gpu, info.fImage, info.fFormat, GrVkImageView::kColor_Type, info.fLevelCount,
            info.fYcbcrConversionInfo);
    if (!imageView) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus = info.fLevelCount > 1 ? GrMipmapStatus::kValid
                                                       : GrMipmapStatus::kNotAllocated;

    GrBackendObjectOwnership ownership = kBorrow_GrWrapOwnership == wrapOwnership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;
    bool isExternal = info.fYcbcrConversionInfo.isValid() &&
                      (info.fYcbcrConversionInfo.fExternalFormat != 0);
    return sk_sp<GrVkTexture>(new GrVkTexture(gpu, dimensions, info, std::move(mutableState),
                                              std::move(imageView), mipmapStatus, ownership,
                                              cacheable, ioType, isExternal));
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTextureView);
}

void GrVkTexture::onRelease() {
    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView.reset();
    }

    fDescSetCache.reset();

    this->releaseImage();

    INHERITED::onRelease();
}

struct GrVkTexture::DescriptorCacheEntry {
    DescriptorCacheEntry(const GrVkDescriptorSet* fDescSet, GrVkGpu* gpu)
            : fDescriptorSet(fDescSet), fGpu(gpu) {}
    ~DescriptorCacheEntry() {
        if (fDescriptorSet) {
            fDescriptorSet->recycle();
        }
    }

    const GrVkDescriptorSet* fDescriptorSet;
    GrVkGpu* fGpu;
};

void GrVkTexture::onAbandon() {
    // we create this and don't hand it off, so we should always destroy it
    if (fTextureView) {
        fTextureView.reset();
    }

    fDescSetCache.reset();

    this->releaseImage();
    INHERITED::onAbandon();
}

GrBackendTexture GrVkTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo, this->getMutableState());
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

const GrVkImageView* GrVkTexture::textureView() {
    return fTextureView.get();
}

const GrVkDescriptorSet* GrVkTexture::cachedSingleDescSet(GrSamplerState state) {
    if (std::unique_ptr<DescriptorCacheEntry>* e = fDescSetCache.find(state)) {
        return (*e)->fDescriptorSet;
    }
    return nullptr;
}

void GrVkTexture::addDescriptorSetToCache(const GrVkDescriptorSet* descSet, GrSamplerState state) {
    SkASSERT(!fDescSetCache.find(state));
    descSet->ref();
    fDescSetCache.insert(state, std::make_unique<DescriptorCacheEntry>(descSet, this->getVkGpu()));
}
