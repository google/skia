/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkTextureRenderTarget.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "src/core/SkMipMap.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 SkISize dimensions,
                                                 int sampleCnt,
                                                 const GrVkImageInfo& info,
                                                 sk_sp<GrVkImageLayout> layout,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageInfo& msaaInfo,
                                                 sk_sp<GrVkImageLayout> msaaLayout,
                                                 const GrVkImageView* colorAttachmentView,
                                                 const GrVkImageView* resolveAttachmentView,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, layout, GrBackendObjectOwnership::kOwned)
        , GrVkTexture(gpu, dimensions, info, layout, texView, mipMapsStatus,
                      GrBackendObjectOwnership::kOwned)
        , GrVkRenderTarget(gpu, dimensions, sampleCnt, info, layout, msaaInfo,
                           std::move(msaaLayout), colorAttachmentView, resolveAttachmentView,
                           GrBackendObjectOwnership::kOwned) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 SkISize dimensions,
                                                 const GrVkImageInfo& info,
                                                 sk_sp<GrVkImageLayout> layout,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageView* colorAttachmentView,
                                                 GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, layout, GrBackendObjectOwnership::kOwned)
        , GrVkTexture(gpu, dimensions, info, layout, texView, mipMapsStatus,
                      GrBackendObjectOwnership::kOwned)
        , GrVkRenderTarget(gpu, dimensions, info, layout, colorAttachmentView,
                           GrBackendObjectOwnership::kOwned) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkISize dimensions,
                                                 int sampleCnt,
                                                 const GrVkImageInfo& info,
                                                 sk_sp<GrVkImageLayout> layout,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageInfo& msaaInfo,
                                                 sk_sp<GrVkImageLayout> msaaLayout,
                                                 const GrVkImageView* colorAttachmentView,
                                                 const GrVkImageView* resolveAttachmentView,
                                                 GrMipMapsStatus mipMapsStatus,
                                                 GrBackendObjectOwnership ownership,
                                                 GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, layout, ownership)
        , GrVkTexture(gpu, dimensions, info, layout, texView, mipMapsStatus, ownership)
        , GrVkRenderTarget(gpu, dimensions, sampleCnt, info, layout, msaaInfo,
                           std::move(msaaLayout), colorAttachmentView, resolveAttachmentView,
                           ownership) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCacheWrapped(cacheable);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 SkISize dimensions,
                                                 const GrVkImageInfo& info,
                                                 sk_sp<GrVkImageLayout> layout,
                                                 const GrVkImageView* texView,
                                                 const GrVkImageView* colorAttachmentView,
                                                 GrMipMapsStatus mipMapsStatus,
                                                 GrBackendObjectOwnership ownership,
                                                 GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, layout, ownership)
        , GrVkTexture(gpu, dimensions, info, layout, texView, mipMapsStatus, ownership)
        , GrVkRenderTarget(gpu, dimensions, info, layout, colorAttachmentView, ownership) {
    this->registerWithCacheWrapped(cacheable);
}

namespace {
struct Views {
    const GrVkImageView* imageView = nullptr;
    const GrVkImageView* colorAttachmentView = nullptr;
    const GrVkImageView* resolveAttachmentView = nullptr;
    GrVkImageInfo msInfo;
    sk_sp<GrVkImageLayout> msLayout;
};
}  // anonymous namespace

static Views create_views(GrVkGpu* gpu, SkISize dimensions, int sampleCnt,
                          const GrVkImageInfo& info) {
    VkImage image = info.fImage;
    // Create the texture ImageView
    Views views;
    views.imageView = GrVkImageView::Create(gpu, image, info.fFormat, GrVkImageView::kColor_Type,
                                            info.fLevelCount, info.fYcbcrConversionInfo);
    if (!views.imageView) {
        return {};
    }

    VkFormat pixelFormat = info.fFormat;

    VkImage colorImage;

    // create msaa surface if necessary
    if (sampleCnt > 1) {
        GrVkImage::ImageDesc msImageDesc;
        msImageDesc.fImageType = VK_IMAGE_TYPE_2D;
        msImageDesc.fFormat = pixelFormat;
        msImageDesc.fWidth = dimensions.fWidth;
        msImageDesc.fHeight = dimensions.fHeight;
        msImageDesc.fLevels = 1;
        msImageDesc.fSamples = sampleCnt;
        msImageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        msImageDesc.fUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        msImageDesc.fMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (!GrVkImage::InitImageInfo(gpu, msImageDesc, &views.msInfo)) {
            views.imageView->unref();
            return {};
        }

        // Set color attachment image
        colorImage = views.msInfo.fImage;

        // Create resolve attachment view.
        views.resolveAttachmentView =
                GrVkImageView::Create(gpu, image, pixelFormat, GrVkImageView::kColor_Type,
                                      info.fLevelCount, GrVkYcbcrConversionInfo());
        if (!views.resolveAttachmentView) {
            GrVkImage::DestroyImageInfo(gpu, &views.msInfo);
            views.imageView->unref();
            return {};
        }
        views.msLayout.reset(new GrVkImageLayout(views.msInfo.fImageLayout));
    } else {
        // Set color attachment image
        colorImage = info.fImage;
    }

    views.colorAttachmentView = GrVkImageView::Create(
            gpu, colorImage, pixelFormat, GrVkImageView::kColor_Type, 1, GrVkYcbcrConversionInfo());
    if (!views.colorAttachmentView) {
        if (sampleCnt > 1) {
            views.resolveAttachmentView->unref();
            GrVkImage::DestroyImageInfo(gpu, &views.msInfo);
        }
        views.imageView->unref();
        return {};
    }
    return views;
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
        GrVkGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        const GrVkImage::ImageDesc& imageDesc,
        GrMipMapsStatus mipMapsStatus) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }
    sk_sp<GrVkImageLayout> layout(new GrVkImageLayout(info.fImageLayout));

    Views views = create_views(gpu, dimensions, sampleCnt, info);
    if (!views.colorAttachmentView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    if (sampleCnt > 1) {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, info, std::move(layout), views.imageView,
                views.msInfo, std::move(views.msLayout), views.colorAttachmentView,
                views.resolveAttachmentView, mipMapsStatus));
    } else {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(layout), views.imageView,
                views.colorAttachmentView, mipMapsStatus));
    }
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        GrWrapOwnership wrapOwnership,
        GrWrapCacheable cacheable,
        const GrVkImageInfo& info,
        sk_sp<GrVkImageLayout> layout) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrMipMapsStatus mipMapsStatus = info.fLevelCount > 1 ? GrMipMapsStatus::kDirty
                                                         : GrMipMapsStatus::kNotAllocated;

    GrBackendObjectOwnership ownership = kBorrow_GrWrapOwnership == wrapOwnership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;
    Views views = create_views(gpu, dimensions, sampleCnt, info);
    if (!views.colorAttachmentView) {
        return nullptr;
    }
    if (sampleCnt > 1) {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, std::move(layout), views.imageView, views.msInfo,
                std::move(views.msLayout), views.colorAttachmentView, views.resolveAttachmentView,
                mipMapsStatus, ownership, cacheable));
    } else {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, dimensions, info, std::move(layout), views.imageView,
                views.colorAttachmentView, mipMapsStatus, ownership, cacheable));
    }
}

size_t GrVkTextureRenderTarget::onGpuMemorySize() const {
    int numColorSamples = this->numSamples();
    if (numColorSamples > 1) {
        // Add one to account for the resolve VkImage.
        ++numColorSamples;
    }
    const GrCaps& caps = *this->getGpu()->caps();
    return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                  numColorSamples,  // TODO: this still correct?
                                  this->texturePriv().mipMapped());
}
