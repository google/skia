/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkTextureRenderTarget.h"

#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkImageView.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include "src/core/SkMipmap.h"

#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrVkTextureRenderTarget::GrVkTextureRenderTarget(GrVkGpu* gpu,
                                                 skgpu::Budgeted budgeted,
                                                 SkISize dimensions,
                                                 sk_sp<GrVkImage> texture,
                                                 sk_sp<GrVkImage> colorAttachment,
                                                 sk_sp<GrVkImage> resolveAttachment,
                                                 GrMipmapStatus mipmapStatus,
                                                 std::string_view label)
        : GrSurface(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    label)
        , GrVkTexture(gpu, dimensions, std::move(texture), mipmapStatus, label)
        , GrVkRenderTarget(gpu,
                           dimensions,
                           std::move(colorAttachment),
                           std::move(resolveAttachment),
                           CreateType::kFromTextureRT,
                           label) {
    this->registerWithCache(budgeted);
}

GrVkTextureRenderTarget::GrVkTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        sk_sp<GrVkImage> texture,
        sk_sp<GrVkImage> colorAttachment,
        sk_sp<GrVkImage> resolveAttachment,
        GrMipmapStatus mipmapStatus,
        GrWrapCacheable cacheable,
        std::string_view label)
        : GrSurface(gpu,
                    dimensions,
                    texture->isProtected() ? GrProtected::kYes : GrProtected::kNo,
                    label)
        , GrVkTexture(gpu, dimensions, std::move(texture), mipmapStatus, label)
        , GrVkRenderTarget(gpu, dimensions, std::move(colorAttachment),
                           std::move(resolveAttachment), CreateType::kFromTextureRT, label) {
    this->registerWithCacheWrapped(cacheable);
}

bool create_rt_attachments(GrVkGpu* gpu, SkISize dimensions, VkFormat format, int sampleCnt,
                           GrProtected isProtected,
                           sk_sp<GrVkImage> texture,
                           sk_sp<GrVkImage>* colorAttachment,
                           sk_sp<GrVkImage>* resolveAttachment) {
    if (sampleCnt > 1) {
        auto rp = gpu->getContext()->priv().resourceProvider();
        sk_sp<GrAttachment> msaaAttachment = rp->makeMSAAAttachment(
                dimensions, GrBackendFormats::MakeVk(format), sampleCnt, isProtected,
                GrMemoryless::kNo);
        if (!msaaAttachment) {
            return false;
        }
        *colorAttachment = sk_sp<GrVkImage>(static_cast<GrVkImage*>(msaaAttachment.release()));
        *resolveAttachment = std::move(texture);
    } else {
        *colorAttachment = std::move(texture);
    }
    return true;
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeNewTextureRenderTarget(
        GrVkGpu* gpu,
        skgpu::Budgeted budgeted,
        SkISize dimensions,
        VkFormat format,
        uint32_t mipLevels,
        int sampleCnt,
        GrMipmapStatus mipmapStatus,
        GrProtected isProtected,
        std::string_view label) {
    sk_sp<GrVkImage> texture = GrVkImage::MakeTexture(gpu,
                                                      dimensions,
                                                      format,
                                                      mipLevels,
                                                      GrRenderable::kYes,
                                                      /*numSamples=*/1,
                                                      budgeted,
                                                      isProtected);
    if (!texture) {
        return nullptr;
    }

    sk_sp<GrVkImage> colorAttachment;
    sk_sp<GrVkImage> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, format, sampleCnt, isProtected, texture,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);
    return sk_sp<GrVkTextureRenderTarget>(new GrVkTextureRenderTarget(gpu,
                                                                      budgeted,
                                                                      dimensions,
                                                                      std::move(texture),
                                                                      std::move(colorAttachment),
                                                                      std::move(resolveAttachment),
                                                                      mipmapStatus,
                                                                      label));
}

sk_sp<GrVkTextureRenderTarget> GrVkTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrVkGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        GrWrapOwnership wrapOwnership,
        GrWrapCacheable cacheable,
        const GrVkImageInfo& info,
        sk_sp<skgpu::MutableTextureState> mutableState) {
    // Adopted textures require both image and allocation because we're responsible for freeing
    SkASSERT(VK_NULL_HANDLE != info.fImage &&
             (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrAttachment::UsageFlags textureUsageFlags = GrAttachment::UsageFlags::kTexture;
    if (info.fImageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        textureUsageFlags |= GrAttachment::UsageFlags::kColorAttachment;
    }

    sk_sp<GrVkImage> texture = GrVkImage::MakeWrapped(gpu,
                                                      dimensions,
                                                      info,
                                                      std::move(mutableState),
                                                      textureUsageFlags,
                                                      wrapOwnership,
                                                      cacheable,
                                                      "VkImage_MakeWrappedTextureRenderTarget");
    if (!texture) {
        return nullptr;
    }

    sk_sp<GrVkImage> colorAttachment;
    sk_sp<GrVkImage> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, info.fFormat, sampleCnt, info.fProtected, texture,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);

    GrMipmapStatus mipmapStatus =
            info.fLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;

    return sk_sp<GrVkTextureRenderTarget>(
            new GrVkTextureRenderTarget(gpu,
                                        dimensions,
                                        std::move(texture),
                                        std::move(colorAttachment),
                                        std::move(resolveAttachment),
                                        mipmapStatus,
                                        cacheable,
                                        /*label=*/"Vk_MakeWrappedTextureRenderTarget"));
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
                 GrSurface::ComputeSize(this->backendFormat(),
                                        this->dimensions(),
                                        this->numSamples(),
                                        skgpu::Mipmapped::kNo));
    }
#endif
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  1 /*colorSamplesPerPixel*/, this->mipmapped());
}
