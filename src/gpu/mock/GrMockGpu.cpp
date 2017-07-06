/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMockGpu.h"
#include "GrMockBuffer.h"
#include "GrMockCaps.h"
#include "GrMockGpuCommandBuffer.h"
#include "GrMockStencilAttachment.h"
#include "GrMockTexture.h"

GrGpu* GrMockGpu::Create(GrBackendContext backendContext, const GrContextOptions& contextOptions,
                         GrContext* context) {
    static const GrMockOptions kDefaultOptions = GrMockOptions();
    const GrMockOptions* options = reinterpret_cast<const GrMockOptions*>(backendContext);
    if (!options) {
        options = &kDefaultOptions;
    }
    return new GrMockGpu(context, *options, contextOptions);
}

GrGpuCommandBuffer* GrMockGpu::createCommandBuffer(const GrGpuCommandBuffer::LoadAndStoreInfo&,
                                                   const GrGpuCommandBuffer::LoadAndStoreInfo&) {
    return new GrMockGpuCommandBuffer(this);
}

void GrMockGpu::submitCommandBuffer(const GrMockGpuCommandBuffer* cmdBuffer) {
    for (int i = 0; i < cmdBuffer->numDraws(); ++i) {
        fStats.incNumDraws();
    }
}

GrMockGpu::GrMockGpu(GrContext* context, const GrMockOptions& options,
                     const GrContextOptions& contextOptions)
        : INHERITED(context) {
    fCaps.reset(new GrMockCaps(contextOptions, options));
}

sk_sp<GrTexture> GrMockGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                            const SkTArray<GrMipLevel>& texels) {
    bool hasMipLevels = texels.count() > 1;
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc, hasMipLevels));
    }
    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, hasMipLevels));
}

GrBuffer* GrMockGpu::onCreateBuffer(size_t sizeInBytes, GrBufferType type,
                                    GrAccessPattern accessPattern, const void*) {
    return new GrMockBuffer(this, sizeInBytes, type, accessPattern);
}

GrStencilAttachment* GrMockGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                       int width,
                                                                       int height) {
    static constexpr int kBits = 8;
    fStats.incStencilAttachmentCreates();
    return new GrMockStencilAttachment(this, width, height, kBits, rt->numColorSamples());
}
