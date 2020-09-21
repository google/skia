/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnStencilAttachment.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrDawnStencilAttachment::GrDawnStencilAttachment(GrDawnGpu* gpu,
                                                 SkISize dimensions,
                                                 int bits,
                                                 int samples,
                                                 wgpu::Texture texture,
                                                 wgpu::TextureView view)
    : INHERITED(gpu, dimensions, bits, samples, GrProtected::kNo)
    , fTexture(texture)
    , fView(view) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnStencilAttachment* GrDawnStencilAttachment::Create(GrDawnGpu* gpu,
                                                         SkISize dimensions,
                                                         int sampleCnt) {
    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::OutputAttachment;
    desc.size.width = dimensions.width();
    desc.size.height = dimensions.height();
    desc.size.depth = 1;
    desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    wgpu::Texture texture = gpu->device().CreateTexture(&desc);
    if (!texture) {
        return nullptr;
    }
    wgpu::TextureView view = texture.CreateView();
    if (!view) {
        return nullptr;
    }
    return new GrDawnStencilAttachment(gpu, dimensions, 8, sampleCnt, texture, view);
}

GrDawnStencilAttachment::~GrDawnStencilAttachment() {
}

size_t GrDawnStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= 32;
    size *= std::max(1,this->numSamples());
    return static_cast<size_t>(size / 8);
}

void GrDawnStencilAttachment::onRelease() {
    GrStencilAttachment::onRelease();
}

void GrDawnStencilAttachment::onAbandon() {
    GrStencilAttachment::onAbandon();
}

GrDawnGpu* GrDawnStencilAttachment::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}
