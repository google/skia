/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlAttachment::GrMtlAttachment(GrMtlGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags supportedUsages,
                                 const id<MTLTexture> stencilView)
        : GrStencilAttachment(gpu, dimensions, supportedUsages, stencilView.sampleCount,
                              GrProtected::kNo)
        , fStencilView(stencilView) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrMtlAttachment* GrMtlAttachment::Create(GrMtlGpu* gpu,
                                         SkISize dimensions,
                                         UsageFlags supportedUsages,
                                         int sampleCnt,
                                         MTLPixelFormat format) {
    SkASSERT(supportedUsages == UsageFlags::kStencil);

    MTLTextureDescriptor* desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                               width:dimensions.width()
                                                              height:dimensions.height()
                                                           mipmapped:NO];
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.storageMode = MTLStorageModePrivate;
        desc.usage = MTLTextureUsageRenderTarget;
    }
    desc.sampleCount = sampleCnt;
    if (sampleCnt > 1) {
        desc.textureType = MTLTextureType2DMultisample;
    }
    return new GrMtlAttachment(gpu, dimensions, supportedUsages,
                               [gpu->device() newTextureWithDescriptor:desc]);
}

GrMtlAttachment::~GrMtlAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fStencilView);
}

size_t GrMtlAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= GrMtlFormatBytesPerBlock(this->mtlFormat());
    size *= this->numSamples();
    return static_cast<size_t>(size);
}

void GrMtlAttachment::onRelease() {
    fStencilView = nullptr;
    GrStencilAttachment::onRelease();
}

void GrMtlAttachment::onAbandon() {
    fStencilView = nullptr;
    GrStencilAttachment::onAbandon();
}

GrMtlGpu* GrMtlAttachment::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}
