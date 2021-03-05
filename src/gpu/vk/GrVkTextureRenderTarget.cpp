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
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<GrVkAttachment> colorAttachment,
        sk_sp<GrVkAttachment> resolveAttachment,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      GrBackendObjectOwnership::kOwned)
        , GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                           std::move(resolveAttachment), CreateType::kFromTextureRT) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        const GrVkImageInfo& info,
        sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
        sk_sp<const GrVkImageView> texView,
        sk_sp<GrVkAttachment> colorAttachment,
        sk_sp<GrVkAttachment> resolveAttachment,
        GrMipmapStatus mipmapStatus,
        GrBackendObjectOwnership ownership,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrVkTexture(gpu, dimensions, info, mutableState, std::move(texView), mipmapStatus,
                      ownership)
        , GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                           std::move(resolveAttachment), CreateType::kFromTextureRT) {
    this->registerWithCacheWrapped(cacheable);
}

namespace {
struct Views {
    sk_sp<const GrVkImageView> textureView;
    sk_sp<GrVkAttachment> colorAttachment;
    sk_sp<GrVkAttachment> resolveAttachment;
};
}  // anonymous namespace

static Views create_attachments(GrVkGpu* gpu, SkISize dimensions, int sampleCnt,
                                const GrVkImageInfo& info,
                                sk_sp<GrBackendSurfaceMutableStateImpl> mutableState) {
    VkImage image = info.fImage;
    // Create the texture ImageView
    Views views;
    views.textureView = GrVkImageView::Make(gpu, image, info.fFormat, GrVkImageView::kColor_Type,
                                            info.fLevelCount, info.fYcbcrConversionInfo);
    if (!views.textureView) {
        return {};
    }

    // Make the non-msaa attachment which may end up as either the color or resolve view depending
    // on if sampleCnt > 1 or not. The info and mutableState passed in here will always represent
    // the non-msaa image.
    SkASSERT(info.fSampleCount == 1);
    // TODO: Fix this weird wrapping once GrVkTexture and GrVkAttachment merge.
    // Regardless of whether the actual TextureRenderTarget is wrapped or created, we always make a
    // wrapped attachment here. The GrVkTexture will manage the lifetime and possible destruction
    // of the GrVkImage object. So we want the attachment on the GrVkRenderTarget to always be
    // borrowed. In the current system that can lead to overcounting of memory usage when we are
    // including both owned and borrowed memory.
    sk_sp<GrVkAttachment> nonMSAAAttachment =
            GrVkAttachment::MakeWrapped(gpu, dimensions, info, std::move(mutableState),
                                        GrAttachment::UsageFlags::kColorAttachment,
                                        kBorrow_GrWrapOwnership, GrWrapCacheable::kNo);

    if (!nonMSAAAttachment) {
        return {};
    }

    // create msaa surface if necessary
    if (sampleCnt > 1) {
        auto rp = gpu->getContext()->priv().resourceProvider();
        sk_sp<GrAttachment> msaaAttachment = rp->makeMSAAAttachment(
                dimensions, GrBackendFormat::MakeVk(info.fFormat), sampleCnt, info.fProtected);
        if (!msaaAttachment) {
            return {};
        }

        views.colorAttachment =
                sk_sp<GrVkAttachment>(static_cast<GrVkAttachment*>(msaaAttachment.release()));
        views.resolveAttachment = std::move(nonMSAAAttachment);
    } else {
        views.colorAttachment = std::move(nonMSAAAttachment);
    }

    if (!views.colorAttachment) {
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

    Views views = create_attachments(gpu, dimensions, sampleCnt, info, mutableState);
    if (!views.colorAttachment) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }
    return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
            gpu, budgeted, dimensions, info, std::move(mutableState), std::move(views.textureView),
            std::move(views.colorAttachment), std::move(views.resolveAttachment), mipmapStatus));
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
    Views views = create_attachments(gpu, dimensions, sampleCnt, info, mutableState);
    if (!views.colorAttachment) {
        return nullptr;
    }
    return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
            gpu, dimensions, info, std::move(mutableState), std::move(views.textureView),
            std::move(views.colorAttachment), std::move(views.resolveAttachment),
            mipmapStatus, ownership, cacheable));
}

size_t GrVkTextureRenderTarget::onGpuMemorySize() const {
    // The GrTexture[RenderTarget] is built up by a bunch of attachments each of which are their
    // own GrGpuResource. Ideally the GrRenderTarget would not be a GrGpuResource and the GrTexture
    // would just merge with the new GrSurface/Attachment world. Then we could just depend on each
    // attachment to give its own size since we don't have GrGpuResources owning other
    // GrGpuResources. Until we get to that point we need to live in some hybrid world. We will let
    // the msaa and stencil attachments track their own size because they do get cached separately.
    // For all GrTexture* based things we will continue to to use the GrTexture* to report size and
    // the owned attachments will have no size and be uncached.
#ifdef SK_DEBUG
    // The nonMSAA attachment (either color or resolve depending on numSamples should have size of
    // zero since it is a texture attachment.
    SkASSERT(this->nonMSAAAttachment()->gpuMemorySize() == 0);
    if (this->numSamples() > 1) {
        // Msaa attachment should have a valid size
        SkASSERT(this->colorAttachment()->gpuMemorySize() ==
                 GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                        this->numSamples(), GrMipMapped::kNo));
    }
#endif
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  1 /*colorSamplesPerPixel*/, this->mipmapped());
}
