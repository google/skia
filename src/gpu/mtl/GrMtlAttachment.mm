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
        : GrAttachment(gpu, dimensions, supportedUsages, texture.sampleCount, GrMipmapped::kNo,
                       GrProtected::kNo)
        , fTexture(texture) {
    this->registerWithCache(budgeted);
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
                                 /*mipLevels=*/1, textureUsage, storageMode, GrProtected::kNo,
                                 SkBudgeted::kYes);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::Make(GrMtlGpu* gpu,
                                             SkISize dimensions,
                                             UsageFlags attachmentUsages,
                                             int sampleCnt,
                                             MTLPixelFormat format,
                                             uint32_t mipLevels,
                                             int mtlTextureUsage,
                                             int mtlStorageMode,
                                             GrProtected isProtected,
                                             SkBudgeted budgeted) {
    auto desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = format;
    desc.width = dimensions.width();
    desc.height = dimensions.height();
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.usage = mtlTextureUsage;
        desc.storageMode = (MTLStorageMode)mtlStorageMode;
    }
    desc.sampleCount = sampleCnt;
    if (sampleCnt > 1) {
        desc.textureType = MTLTextureType2DMultisample;
    }
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:desc];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (attachmentUsages == UsageFlags::kStencilAttachment) {
        texture.label = @"Stencil";
    }
#endif

    return sk_sp<GrMtlAttachment>(new GrMtlAttachment(gpu, dimensions, attachmentUsages,
                                                      texture, budgeted));
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
