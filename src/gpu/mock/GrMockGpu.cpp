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

int GrMockGpu::NextInternalTextureID() {
    static int gID = 0;
    return sk_atomic_inc(&gID) + 1;
}

int GrMockGpu::NextExternalTextureID() {
    // We use negative ints for the "testing only external textures" so they can easily be
    // identified when debugging.
    static int gID = 0;
    return sk_atomic_dec(&gID) - 1;
}

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
                                            const GrMipLevel texels[], int mipLevelCount) {
    bool hasMipLevels = mipLevelCount > 1;
    GrMockTextureInfo info;
    info.fID = NextInternalTextureID();
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return sk_sp<GrTexture>(
                new GrMockTextureRenderTarget(this, budgeted, desc, hasMipLevels, info));
    }
    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, hasMipLevels, info));
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

GrBackendObject GrMockGpu::createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                           GrPixelConfig config, bool isRT) {
    auto info = new GrMockTextureInfo;
    info->fID = NextExternalTextureID();
    fOutstandingTestingOnlyTextureIDs.add(info->fID);
    return reinterpret_cast<GrBackendObject>(info);
}

bool GrMockGpu::isTestingOnlyBackendTexture(GrBackendObject object) const {
    return fOutstandingTestingOnlyTextureIDs.contains(
            reinterpret_cast<const GrMockTextureInfo*>(object)->fID);
}

void GrMockGpu::deleteTestingOnlyBackendTexture(GrBackendObject object, bool abandonTexture) {
    auto info = reinterpret_cast<const GrMockTextureInfo*>(object);
    fOutstandingTestingOnlyTextureIDs.remove(info->fID);
    delete info;
}
