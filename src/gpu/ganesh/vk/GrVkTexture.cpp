/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/ganesh/vk/GrVkTexture.h"

#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/vk/GrVkBackendSurfacePriv.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSet.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImageView.h"
#include "src/gpu/ganesh/vk/GrVkTextureRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         skgpu::Budgeted budgeted,
                         SkISize dimensions,
                         sk_sp<GrVkImage> texture,
                         GrMipmapStatus mipmapStatus,
                         std::string_view label)
        : GrSurface(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    label)
        , GrTexture(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    GrTextureType::k2D,
                    mipmapStatus,
                    label)
        , fTexture(std::move(texture))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == fTexture->mipLevels()));
    // We don't support creating external GrVkTextures
    SkASSERT(!fTexture->ycbcrConversionInfo().isValid() ||
             !fTexture->ycbcrConversionInfo().fExternalFormat);
    SkASSERT(SkToBool(fTexture->vkUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT));
    this->registerWithCache(budgeted);
    if (skgpu::VkFormatIsCompressed(fTexture->imageFormat())) {
        this->setReadOnly();
    }
}

GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkISize dimensions,
                         sk_sp<GrVkImage> texture,
                         GrMipmapStatus mipmapStatus,
                         GrWrapCacheable cacheable,
                         GrIOType ioType,
                         bool isExternal,
                         std::string_view label)
        : GrSurface(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    label)
        , GrTexture(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    isExternal ? GrTextureType::kExternal : GrTextureType::k2D,
                    mipmapStatus,
                    label)
        , fTexture(std::move(texture))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == fTexture->mipLevels()));
    SkASSERT(SkToBool(fTexture->vkUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrVkTexture::GrVkTexture(GrVkGpu* gpu,
                         SkISize dimensions,
                         sk_sp<GrVkImage> texture,
                         GrMipmapStatus mipmapStatus,
                         std::string_view label)
        : GrSurface(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    label)
        , GrTexture(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    GrTextureType::k2D,
                    mipmapStatus,
                    label)
        , fTexture(std::move(texture))
        , fDescSetCache(kMaxCachedDescSets) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == fTexture->mipLevels()));
    // Since this ctor is only called from GrVkTextureRenderTarget, we can't have a ycbcr conversion
    // since we don't support that on render targets.
    SkASSERT(!fTexture->ycbcrConversionInfo().isValid());
    SkASSERT(SkToBool(fTexture->vkUsageFlags() & VK_IMAGE_USAGE_SAMPLED_BIT));
}

sk_sp<GrVkTexture> GrVkTexture::MakeNewTexture(GrVkGpu* gpu,
                                               skgpu::Budgeted budgeted,
                                               SkISize dimensions,
                                               VkFormat format,
                                               uint32_t mipLevels,
                                               GrProtected isProtected,
                                               GrMipmapStatus mipmapStatus,
                                               std::string_view label) {
    sk_sp<GrVkImage> texture = GrVkImage::MakeTexture(
            gpu, dimensions, format, mipLevels, GrRenderable::kNo, /*numSamples=*/1, budgeted,
            isProtected);

    if (!texture) {
        return nullptr;
    }
    return sk_sp<GrVkTexture>(new GrVkTexture(
            gpu, budgeted, dimensions, std::move(texture), mipmapStatus, label));
}

sk_sp<GrVkTexture> GrVkTexture::MakeWrappedTexture(
        GrVkGpu* gpu, SkISize dimensions, GrWrapOwnership wrapOwnership, GrWrapCacheable cacheable,
        GrIOType ioType, const GrVkImageInfo& info,
        sk_sp<skgpu::MutableTextureStateRef> mutableState) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    sk_sp<GrVkImage> texture = GrVkImage::MakeWrapped(gpu,
                                                      dimensions,
                                                      info,
                                                      std::move(mutableState),
                                                      GrAttachment::UsageFlags::kTexture,
                                                      wrapOwnership,
                                                      cacheable,
                                                      "VkImage_MakeWrappedTexture");
    if (!texture) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus = info.fLevelCount > 1 ? GrMipmapStatus::kValid
                                                       : GrMipmapStatus::kNotAllocated;

    bool isExternal = info.fYcbcrConversionInfo.isValid() &&
                      (info.fYcbcrConversionInfo.fExternalFormat != 0);
    isExternal |= (info.fImageTiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);
    return sk_sp<GrVkTexture>(new GrVkTexture(gpu,
                                              dimensions,
                                              std::move(texture),
                                              mipmapStatus,
                                              cacheable,
                                              ioType,
                                              isExternal,
                                              /*label=*/"Vk_MakeWrappedTexture"));
}

GrVkTexture::~GrVkTexture() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fTexture);
}

void GrVkTexture::onRelease() {
    fTexture.reset();

    fDescSetCache.reset();

    GrTexture::onRelease();
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
    fTexture.reset();

    fDescSetCache.reset();

    GrTexture::onAbandon();
}

GrBackendTexture GrVkTexture::getBackendTexture() const {
    return GrBackendTextures::MakeVk(fTexture->width(),
                                     fTexture->height(),
                                     fTexture->vkImageInfo(),
                                     fTexture->getMutableState());
}

GrVkGpu* GrVkTexture::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

const GrVkImageView* GrVkTexture::textureView() { return fTexture->textureView(); }

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
