/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
        GrMtlGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        MTLPixelFormat format,
        uint32_t mipLevels,
        GrMipmapStatus mipmapStatus) {
    sk_sp<GrMtlAttachment> texture =
            GrMtlAttachment::MakeTexture(gpu, dimensions, format, mipLevels, GrRenderable::kYes,
                                         /*numSamples=*/1, budgeted);
    if (!texture) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) &
                 texture->mtlTexture().usage);
    }

    if (sampleCnt > 1) {
        // TODO: create in resourceprovider
        sk_sp<GrMtlAttachment> colorAttachment =
                GrMtlAttachment::MakeMSAA(gpu, dimensions, sampleCnt, texture->mtlFormat());
        if (!colorAttachment) {
            return nullptr;
        }
        sk_sp<GrMtlAttachment> resolveAttachment = texture;
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, budgeted, dimensions, std::move(texture), std::move(colorAttachment),
                std::move(resolveAttachment), mipmapStatus));
    } else {
        sk_sp<GrMtlAttachment> colorAttachment = texture;
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, budgeted, dimensions, std::move(texture),
                                             std::move(colorAttachment), nullptr, mipmapStatus));
    }
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

    if (sampleCnt > 1) {
        // TODO: create in resourceprovider
        sk_sp<GrMtlAttachment> colorAttachment =
                GrMtlAttachment::MakeMSAA(gpu, dimensions, sampleCnt, texture.pixelFormat);
        if (!colorAttachment) {
            return nullptr;
        }
        sk_sp<GrMtlAttachment> resolveAttachment = textureAttachment;
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, dimensions, std::move(textureAttachment), std::move(colorAttachment),
                std::move(resolveAttachment), mipmapStatus, cacheable));
    } else {
        sk_sp<GrMtlAttachment> colorAttachment = textureAttachment;
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, dimensions, std::move(textureAttachment),
                                             std::move(colorAttachment), nullptr, mipmapStatus,
                                             cacheable));
    }
}

GR_NORETAIN_END
