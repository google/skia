/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

GrMtlStencilAttachment::GrMtlStencilAttachment(GrMtlGpu* gpu,
                                               const Format& format,
                                               const id<MTLTexture> stencilView)
    : GrStencilAttachment(gpu, stencilView.width, stencilView.height, format.fStencilBits,
                          stencilView.sampleCount)
    , fFormat(format)
    , fStencilView(stencilView) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrMtlStencilAttachment* GrMtlStencilAttachment::Create(GrMtlGpu* gpu,
                                                       int width,
                                                       int height,
                                                       int sampleCnt,
                                                       const Format& format) {
    MTLTextureDescriptor* desc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format.fInternalFormat
                                                           width:width
                                                          height:height
                                                       mipmapped:NO];
    desc.resourceOptions = MTLResourceStorageModePrivate;
    desc.usage = MTLTextureUsageRenderTarget;
    return new GrMtlStencilAttachment(gpu, format, [gpu->device() newTextureWithDescriptor:desc]);
}

GrMtlStencilAttachment::~GrMtlStencilAttachment() {
    // should have been released or abandoned first
    SkASSERT(!fStencilView);
}

size_t GrMtlStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= fFormat.fTotalBits;
    size *= this->numSamples();
    return static_cast<size_t>(size / 8);
}

void GrMtlStencilAttachment::onRelease() {
    fStencilView = nullptr;
    GrStencilAttachment::onRelease();
}

void GrMtlStencilAttachment::onAbandon() {
    fStencilView = nullptr;
    GrStencilAttachment::onAbandon();
}

GrMtlGpu* GrMtlStencilAttachment::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}
