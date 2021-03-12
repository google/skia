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
        sk_sp<GrVkAttachment> texture,
        sk_sp<GrVkAttachment> colorAttachment,
        sk_sp<GrVkAttachment> resolveAttachment,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, texture->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , GrVkTexture(gpu, dimensions, std::move(texture), mipmapStatus)
        , GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                           std::move(resolveAttachment), CreateType::kFromTextureRT) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        sk_sp<GrVkAttachment> texture,
        sk_sp<GrVkAttachment> colorAttachment,
        sk_sp<GrVkAttachment> resolveAttachment,
        GrMipmapStatus mipmapStatus,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, texture->isProtected() ? GrProtected::kYes : GrProtected::kNo)
        , GrVkTexture(gpu, dimensions, std::move(texture), mipmapStatus)
        , GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                           std::move(resolveAttachment), CreateType::kFromTextureRT) {
    this->registerWithCacheWrapped(cacheable);
}

bool create_rt_attachments(GrVkGpu* gpu, SkISize dimensions, VkFormat format, int sampleCnt,
                           GrProtected isProtected, sk_sp<GrVkAttachment> texture,
                           sk_sp<GrVkAttachment>* colorAttachment,
                           sk_sp<GrVkAttachment>* resolveAttachment) {
    if (sampleCnt > 1) {
        auto rp = gpu->getContext()->priv().resourceProvider();
        sk_sp<GrAttachment> msaaAttachment = rp->makeMSAAAttachment(
                dimensions, GrBackendFormat::MakeVk(format), sampleCnt, isProtected);
        if (!msaaAttachment) {
            return false;
        }
        *colorAttachment =
                sk_sp<GrVkAttachment>(static_cast<GrVkAttachment*>(msaaAttachment.release()));
        *resolveAttachment = std::move(texture);
    } else {
        *colorAttachment = std::move(texture);
    }
    return true;
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
        GrVkGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        VkFormat format,
        uint32_t mipLevels,
        int sampleCnt,
        GrMipmapStatus mipmapStatus,
        GrProtected isProtected) {
    sk_sp<GrVkAttachment> texture =
            GrVkAttachment::MakeTexture(gpu, dimensions, format, mipLevels, GrRenderable::kYes,
                                        /*numSamples=*/1, budgeted, isProtected);
    if (!texture) {
        return nullptr;
    }

    sk_sp<GrVkAttachment> colorAttachment;
    sk_sp<GrVkAttachment> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, format, sampleCnt, isProtected, texture,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);
    return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
            gpu, budgeted, dimensions, std::move(texture), std::move(colorAttachment),
            std::move(resolveAttachment), mipmapStatus));
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

    GrAttachment::UsageFlags textureUsageFlags = GrAttachment::UsageFlags::kTexture;
    if (info.fImageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        textureUsageFlags |= GrAttachment::UsageFlags::kColorAttachment;
    }

    sk_sp<GrVkAttachment> texture =
            GrVkAttachment::MakeWrapped(gpu, dimensions, info, std::move(mutableState),
                                        textureUsageFlags, wrapOwnership, cacheable);
    if (!texture) {
        return nullptr;
    }

    sk_sp<GrVkAttachment> colorAttachment;
    sk_sp<GrVkAttachment> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, info.fFormat, sampleCnt, info.fProtected, texture,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);

    GrMipmapStatus mipmapStatus =
            info.fLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;

    return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(
            gpu, dimensions, std::move(texture), std::move(colorAttachment),
            std::move(resolveAttachment), mipmapStatus, cacheable));
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
