/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnAttachment.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

GrDawnAttachment::GrDawnAttachment(GrDawnGpu* gpu,
                                   SkISize dimensions,
                                   UsageFlags supportedUsages,
                                   int samples,
                                   wgpu::Texture texture,
                                   wgpu::TextureView view)
        : INHERITED(gpu, dimensions, supportedUsages, samples, GrMipmapped::kNo, GrProtected::kNo)
        , fTexture(texture)
        , fView(view) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrDawnAttachment> GrDawnAttachment::MakeStencil(GrDawnGpu* gpu,
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
    return sk_sp<GrDawnAttachment>(
            new GrDawnAttachment(gpu, dimensions, UsageFlags::kStencilAttachment, sampleCnt,
                                 texture, view));
}

GrDawnAttachment::~GrDawnAttachment() {}

void GrDawnAttachment::onRelease() { GrAttachment::onRelease(); }

void GrDawnAttachment::onAbandon() { GrAttachment::onAbandon(); }

GrDawnGpu* GrDawnAttachment::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}
