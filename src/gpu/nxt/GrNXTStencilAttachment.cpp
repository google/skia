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
    dawn::Texture texture = gpu->device().CreateTextureBuilder()
        .SetDimension(dawn::TextureDimension::e2D)
        .SetExtent(width, height, 1)
        .SetFormat(dawn::TextureFormat::D32FloatS8Uint)
        .SetMipLevels(1)
        .SetAllowedUsage(dawn::TextureUsageBit::OutputAttachment)
        .GetResult();
    if (!texture) {
        return nullptr;
    }
    dawn::TextureView view = texture.CreateTextureViewBuilder().GetResult();
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
