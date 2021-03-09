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

GrMtlAttachment::GrMtlAttachment(GrMtlGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags supportedUsages,
                                 const id<MTLTexture> view)
        : GrAttachment(gpu, dimensions, supportedUsages, view.sampleCount, GrMipmapped::kNo,
                       GrProtected::kNo)
        , fView(view) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrMtlAttachment> GrMtlAttachment::MakeStencil(GrMtlGpu* gpu,
                                                    SkISize dimensions,
                                                    int sampleCnt,
                                                    MTLPixelFormat format) {
    auto desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = format;
    desc.width = dimensions.width();
    desc.height = dimensions.height();
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.storageMode = MTLStorageModePrivate;
        desc.usage = MTLTextureUsageRenderTarget;
    }
    desc.sampleCount = sampleCnt;
    if (sampleCnt > 1) {
        desc.textureType = MTLTextureType2DMultisample;
    }
    return sk_sp<GrMtlAttachment>(
            new GrMtlAttachment(gpu, dimensions, UsageFlags::kStencilAttachment,
                                [gpu->device() newTextureWithDescriptor:desc]));
}

GrMtlAttachment::~GrMtlAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fView);
}

void GrMtlAttachment::onRelease() {
    fView = nullptr;
    GrAttachment::onRelease();
}

void GrMtlAttachment::onAbandon() {
    fView = nullptr;
    GrAttachment::onAbandon();
}

GrMtlGpu* GrMtlAttachment::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}
