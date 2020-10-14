/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkTextureRenderTarget.h"

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/vk/GrVkAttachment.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkUtil.h"

#include "src/core/SkMipmap.h"

#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<GrVkAttachment> msaaAttachment,
        sk_sp<const GrVkImageView> colorAttachmentView,
        sk_sp<const GrVkImageView> resolveAttachmentView,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, mutableState, GrBackendObjectOwnership::kOwned)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      GrBackendObjectOwnership::kOwned)
        , GrVkRenderTarget(gpu, dimensions, sampleCnt, info, std::move(mutableState),
                           std::move(msaaAttachment), std::move(colorAttachmentView),
                           std::move(resolveAttachmentView), GrBackendObjectOwnership::kOwned) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<const GrVkImageView> colorAttachmentView,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, mutableState, GrBackendObjectOwnership::kOwned)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      GrBackendObjectOwnership::kOwned)
        , GrVkRenderTarget(gpu, dimensions, info, std::move(mutableState),
                           std::move(colorAttachmentView), GrBackendObjectOwnership::kOwned) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<GrVkAttachment> msaaAttachment,
        sk_sp<const GrVkImageView> colorAttachmentView,
        sk_sp<const GrVkImageView> resolveAttachmentView,
        GrMipmapStatus mipmapStatus,
        GrBackendObjectOwnership ownership,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, mutableState, ownership)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      ownership)
        , GrVkRenderTarget(gpu, dimensions, sampleCnt, info, std::move(mutableState),
                           std::move(msaaAttachment), std::move(colorAttachmentView),
                           std::move(resolveAttachmentView), ownership) {
    this->registerWithCacheWrapped(cacheable);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<const GrVkImageView> colorAttachmentView,
        GrMipmapStatus mipmapStatus,
        GrBackendObjectOwnership ownership,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkImage(gpu, info, mutableState, ownership)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      ownership)
        , GrVkRenderTarget(gpu, dimensions, info, std::move(mutableState),
                           std::move(colorAttachmentView), ownership) {
    this->registerWithCacheWrapped(cacheable);
}

namespace {
struct Views {
    sk_sp<const GrVkImageView> imageView;
    sk_sp<const GrVkImageView> colorAttachmentView;
    sk_sp<const GrVkImageView> resolveAttachmentView;
    sk_sp<GrVkAttachment> msaaAttachment;
};
}  // anonymous namespace

static Views create_views(GrVkGpu* gpu, SkISize dimensions, int sampleCnt,
                          const GrVkImageInfo& info) {
    VkImage image = info.fImage;
    // Create the texture ImageView
    Views views;
    views.imageView = GrVkImageView::Make(gpu, image, info.fFormat, GrVkImageView::kColor_Type,
                                          info.fLevelCount, info.fYcbcrConversionInfo);
    if (!views.imageView) {
        return {};
    }

    VkFormat pixelFormat = info.fFormat;

    // create msaa surface if necessary
    if (sampleCnt > 1) {
        auto rp = gpu->getContext()->priv().resourceProvider();
        sk_sp<GrAttachment> msaaAttachment =
                rp->makeMSAAAttachment(dimensions, GrBackendFormat::MakeVk(info.fFormat),
                                         sampleCnt, info.fProtected);
        if (!msaaAttachment) {
            return {};
        }

        views.msaaAttachment = sk_sp<GrVkAttachment>(
                static_cast<GrVkAttachment*>(msaaAttachment.release()));

        views.colorAttachmentView = sk_ref_sp<const GrVkImageView>(views.msaaAttachment->view());

        // Create resolve attachment view.
        views.resolveAttachmentView =
                GrVkImageView::Make(gpu, image, pixelFormat, GrVkImageView::kColor_Type,
                                    info.fLevelCount, GrVkYcbcrConversionInfo());
        if (!views.resolveAttachmentView) {
            return {};
        }
    } else {
        views.colorAttachmentView = GrVkImageView::Make(
                gpu, info.fImage, pixelFormat, GrVkImageView::kColor_Type, 1,
                GrVkYcbcrConversionInfo());
    }

    if (!views.colorAttachmentView) {
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
        GrMipmapStatus mipmapStatus) {
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    SkASSERT(imageDesc.fUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT);

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }
    sk_sp<GrBackendSurfaceMutableStateImpl> mutableState(
            new GrBackendSurfaceMutableStateImpl(info.fImageLayout, info.fCurrentQueueFamily));

    Views views = create_views(gpu, dimensions, sampleCnt, info);
    if (!views.colorAttachmentView) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    if (sampleCnt > 1) {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, info, std::move(mutableState),
                std::move(views.imageView), std::move(views.msaaAttachment),
                std::move(views.colorAttachmentView), std::move(views.resolveAttachmentView),
                mipmapStatus));
    } else {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(mutableState),
                std::move(views.imageView), std::move(views.colorAttachmentView), mipmapStatus));
    }
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        GrWrapOwnership wrapOwnership,
        GrWrapCacheable cacheable,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrMipmapStatus mipmapStatus = info.fLevelCount > 1 ? GrMipmapStatus::kDirty
                                                       : GrMipmapStatus::kNotAllocated;

    GrBackendObjectOwnership ownership = kBorrow_GrWrapOwnership == wrapOwnership
            ? GrBackendObjectOwnership::kBorrowed : GrBackendObjectOwnership::kOwned;
    Views views = create_views(gpu, dimensions, sampleCnt, info);
    if (!views.colorAttachmentView) {
        return nullptr;
    }
    if (sampleCnt > 1) {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, std::move(mutableState),
                std::move(views.imageView), std::move(views.msaaAttachment),
                std::move(views.colorAttachmentView), std::move(views.resolveAttachmentView),
                mipmapStatus, ownership, cacheable));
    } else {
        return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
                gpu, dimensions, info, std::move(mutableState), std::move(views.imageView),
                std::move(views.colorAttachmentView), mipmapStatus, ownership, cacheable));
    }
}

size_t GrVkTextureRenderTarget::onGpuMemorySize() const {
    int numColorSamples = this->numSamples();
    if (numColorSamples > 1) {
        // Add one to account for the resolve VkImage.
        ++numColorSamples;
    }
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  numColorSamples,  // TODO: this still correct?
                                  this->mipmapped());
}
