/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlAttachment.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlAttachment::GrMtlAttachment(GrMtlGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags supportedUsages,
                                 id<MTLTexture> texture,
                                 SkBudgeted budgeted)
        : GrAttachment(gpu, dimensions, supportedUsages, texture.sampleCount,
                       texture.mipmapLevelCount > 1 ? GrMipmapped::kYes : GrMipmapped::kNo,
                       GrProtected::kNo)
        , fTexture(texture) {
    this->registerWithCache(budgeted);
}

GrMtlAttachment::GrMtlAttachment(GrMtlGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags supportedUsages,
                                 id<MTLTexture> texture,
                                 GrWrapCacheable cacheable)
        : GrAttachment(gpu, dimensions, supportedUsages, texture.sampleCount,
                       texture.mipmapLevelCount > 1 ? GrMipmapped::kYes : GrMipmapped::kNo,
                       GrProtected::kNo)
        , fTexture(texture) {
    this->registerWithCacheWrapped(cacheable);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::MakeStencil(GrMtlGpu* gpu,
                                                    SkISize dimensions,
                                                    int sampleCnt,
                                                    MTLPixelFormat format) {
    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage = MTLTextureUsageRenderTarget;
        storageMode = MTLStorageModePrivate;
    }
    return GrMtlAttachment::Make(gpu, dimensions, UsageFlags::kStencilAttachment, sampleCnt, format,
                                 /*mipLevels=*/1, textureUsage, storageMode, SkBudgeted::kYes);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::MakeMSAA(GrMtlGpu* gpu,
                                                 SkISize dimensions,
                                                 int sampleCnt,
                                                 MTLPixelFormat format) {
    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        storageMode = MTLStorageModePrivate;
    }
    return GrMtlAttachment::Make(gpu, dimensions, UsageFlags::kColorAttachment, sampleCnt, format,
                                 /*mipLevels=*/1, textureUsage, storageMode, SkBudgeted::kYes);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::MakeTexture(GrMtlGpu* gpu,
                                                    SkISize dimensions,
                                                    MTLPixelFormat format,
                                                    uint32_t mipLevels,
                                                    GrRenderable renderable,
                                                    int numSamples,
                                                    SkBudgeted budgeted) {
    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage = MTLTextureUsageShaderRead;
        storageMode = MTLStorageModePrivate;
    }
    UsageFlags usageFlags = UsageFlags::kTexture;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= UsageFlags::kColorAttachment;
        if (@available(macOS 10.11, iOS 9.0, *)) {
            textureUsage |= MTLTextureUsageRenderTarget;
        }
    }

    return GrMtlAttachment::Make(gpu, dimensions, usageFlags, numSamples, format, mipLevels,
                                 textureUsage, storageMode, budgeted);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::Make(GrMtlGpu* gpu,
                                             SkISize dimensions,
                                             UsageFlags attachmentUsages,
                                             int sampleCnt,
                                             MTLPixelFormat format,
                                             uint32_t mipLevels,
                                             int mtlTextureUsage,
                                             int mtlStorageMode,
                                             SkBudgeted budgeted) {
    auto desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType = (sampleCnt > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
    desc.pixelFormat = format;
    desc.width = dimensions.width();
    desc.height = dimensions.height();
    desc.depth = 1;
    desc.mipmapLevelCount = mipLevels;
    desc.sampleCount = sampleCnt;
    desc.arrayLength = 1;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.usage = mtlTextureUsage;
        desc.storageMode = (MTLStorageMode)mtlStorageMode;
    }
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:desc];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (attachmentUsages == UsageFlags::kStencilAttachment) {
        texture.label = @"Stencil";
    } else if (SkToBool(attachmentUsages & UsageFlags::kColorAttachment)) {
        if (sampleCnt > 1) {
            if (SkToBool(attachmentUsages & UsageFlags::kTexture)) {
                texture.label = @"MSAA TextureRenderTarget";
            } else {
                texture.label = @"MSAA RenderTarget";
            }
        } else {
            if (SkToBool(attachmentUsages & UsageFlags::kTexture)) {
                texture.label = @"TextureRenderTarget";
            } else {
                texture.label = @"RenderTarget";
            }
        }
    } else {
        SkASSERT(attachmentUsages == UsageFlags::kTexture);
        texture.label = @"Texture";
    }
#endif

    return sk_sp<GrMtlAttachment>(new GrMtlAttachment(gpu, dimensions, attachmentUsages,
                                                      texture, budgeted));
}

sk_sp<GrMtlAttachment> GrMtlAttachment::MakeWrapped(
        GrMtlGpu* gpu,
        SkISize dimensions,
        id<MTLTexture> texture,
        UsageFlags attachmentUsages,
        GrWrapCacheable cacheable) {

    return sk_sp<GrMtlAttachment>(new GrMtlAttachment(gpu, dimensions, attachmentUsages, texture,
                                                      cacheable));
}

GrMtlAttachment::~GrMtlAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fTexture);
}

void GrMtlAttachment::onRelease() {
    fTexture = nil;
    GrAttachment::onRelease();
}

void GrMtlAttachment::onAbandon() {
    fTexture = nil;
    GrAttachment::onAbandon();
}

GrMtlGpu* GrMtlAttachment::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GR_NORETAIN_END
