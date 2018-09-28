/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTStencilAttachment.h"

#include "GrNXTGpu.h"
#include "GrNXTUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrNXTStencilAttachment::GrNXTStencilAttachment(GrNXTGpu* gpu,
                                               int width,
                                               int height,
                                               int bits,
                                               int samples,
                                               dawn::Texture texture,
                                               dawn::TextureView view)
    : INHERITED(gpu, width, height, bits, samples)
    , fTexture(texture.Clone())
    , fView(view.Clone()) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrNXTStencilAttachment* GrNXTStencilAttachment::Create(GrNXTGpu* gpu,
                                                       int width,
                                                       int height,
                                                       int sampleCnt) {
    dawn::TextureDescriptor desc;
    desc.usage = dawn::TextureUsageBit::OutputAttachment;
    desc.dimension = dawn::TextureDimension::e2D;
    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.arrayLayer = 1;
    desc.format = dawn::TextureFormat::D32FloatS8Uint;
    desc.mipLevel = 1;
    dawn::Texture texture = gpu->device().CreateTexture(&desc);
    if (!texture) {
        return nullptr;
    }
    dawn::TextureView view = texture.CreateDefaultTextureView();
    if (!view) {
        return nullptr;
    }
    return new GrNXTStencilAttachment(gpu, width, height, 8, sampleCnt, texture.Clone(), view.Clone());
}

GrNXTStencilAttachment::~GrNXTStencilAttachment() {
}

size_t GrNXTStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= 32;
    size *= SkTMax(1,this->numSamples());
    return static_cast<size_t>(size / 8);
}

void GrNXTStencilAttachment::onRelease() {
    GrStencilAttachment::onRelease();
}

void GrNXTStencilAttachment::onAbandon() {
    GrStencilAttachment::onAbandon();
}

GrNXTGpu* GrNXTStencilAttachment::getNXTGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrNXTGpu*>(this->getGpu());
}
