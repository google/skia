/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   sk_sp<GrMtlAttachment> texture,
                                                   sk_sp<GrMtlAttachment> colorAttachment,
                                                   sk_sp<GrMtlAttachment> resolveAttachment,
                                                   GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, std::move(texture), mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, std::move(colorAttachment),
                            std::move(resolveAttachment)) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkISize dimensions,
                                                   sk_sp<GrMtlAttachment> texture,
                                                   sk_sp<GrMtlAttachment> colorAttachment,
                                                   sk_sp<GrMtlAttachment> resolveAttachment,
                                                   GrMipmapStatus mipmapStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, std::move(texture), mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, std::move(colorAttachment),
                            std::move(resolveAttachment)) {
    this->registerWithCacheWrapped(cacheable);
}

bool create_rt_attachments(GrMtlGpu* gpu, SkISize dimensions, MTLPixelFormat format, int sampleCnt,
                           sk_sp<GrMtlAttachment> texture, sk_sp<GrMtlAttachment>* colorAttachment,
                           sk_sp<GrMtlAttachment>* resolveAttachment) {
    if (sampleCnt > 1) {
        auto rp = gpu->getContext()->priv().resourceProvider();
        sk_sp<GrAttachment> msaaAttachment = rp->makeMSAAAttachment(
                dimensions, GrBackendFormat::MakeMtl(format), sampleCnt, GrProtected::kNo,
                GrMemoryless::kNo);
        if (!msaaAttachment) {
            return false;
        }
        *colorAttachment =
                sk_sp<GrMtlAttachment>(static_cast<GrMtlAttachment*>(msaaAttachment.release()));
        *resolveAttachment = std::move(texture);
    } else {
        *colorAttachment = std::move(texture);
    }
    return true;
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
        GrMtlGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        MTLPixelFormat format,
        uint32_t mipLevels,
        GrMipmapStatus mipmapStatus) {
    sk_sp<GrMtlAttachment> textureAttachment =
            GrMtlAttachment::MakeTexture(gpu, dimensions, format, mipLevels, GrRenderable::kYes,
                                         /*numSamples=*/1, budgeted);
    if (!textureAttachment) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) &
                 textureAttachment->mtlTexture().usage);
    }

    sk_sp<GrMtlAttachment> colorAttachment;
    sk_sp<GrMtlAttachment> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, format, sampleCnt, textureAttachment,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);

    return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
            gpu, budgeted, dimensions, std::move(textureAttachment), std::move(colorAttachment),
            std::move(resolveAttachment), mipmapStatus));
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrMtlGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        id<MTLTexture> texture,
        GrWrapCacheable cacheable) {
    SkASSERT(nil != texture);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);
    }
    GrMipmapStatus mipmapStatus = texture.mipmapLevelCount > 1
                                            ? GrMipmapStatus::kDirty
                                            : GrMipmapStatus::kNotAllocated;
    GrAttachment::UsageFlags textureUsageFlags = GrAttachment::UsageFlags::kTexture |
                                                 GrAttachment::UsageFlags::kColorAttachment;
    sk_sp<GrMtlAttachment> textureAttachment =
            GrMtlAttachment::MakeWrapped(gpu, dimensions, texture, textureUsageFlags, cacheable);
    if (!textureAttachment) {
        return nullptr;
    }

    sk_sp<GrMtlAttachment> colorAttachment;
    sk_sp<GrMtlAttachment> resolveAttachment;
    if (!create_rt_attachments(gpu, dimensions, texture.pixelFormat, sampleCnt, textureAttachment,
                               &colorAttachment, &resolveAttachment)) {
        return nullptr;
    }
    SkASSERT(colorAttachment);
    SkASSERT(sampleCnt == 1 || resolveAttachment);

    return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
            gpu, dimensions, std::move(textureAttachment), std::move(colorAttachment),
            std::move(resolveAttachment), mipmapStatus, cacheable));
}

size_t GrMtlTextureRenderTarget::onGpuMemorySize() const {
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

GR_NORETAIN_END
