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
    return Create(reinterpret_cast<const GrMockOptions*>(backendContext), contextOptions, context);
}

GrGpu* GrMockGpu::Create(const GrMockOptions* mockOptions, const GrContextOptions& contextOptions,
                         GrContext* context) {
    static const GrMockOptions kDefaultOptions = GrMockOptions();
    if (!mockOptions) {
        mockOptions = &kDefaultOptions;
    }
    return new GrMockGpu(context, *mockOptions, contextOptions);
}


GrGpuRTCommandBuffer* GrMockGpu::createCommandBuffer(
                                            GrRenderTarget* rt, GrSurfaceOrigin origin,
                                            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) {
    return new GrMockGpuRTCommandBuffer(this, rt, origin);
}

GrGpuTextureCommandBuffer* GrMockGpu::createCommandBuffer(GrTexture* texture,
                                                          GrSurfaceOrigin origin) {
    return new GrMockGpuTextureCommandBuffer(texture, origin);
}


void GrMockGpu::submitCommandBuffer(const GrMockGpuRTCommandBuffer* cmdBuffer) {
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
    GrMipMapsStatus mipMapsStatus = mipLevelCount > 1 ? GrMipMapsStatus::kValid
                                                      : GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo info;
    info.fID = NextInternalTextureID();
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return sk_sp<GrTexture>(
                new GrMockTextureRenderTarget(this, budgeted, desc, mipMapsStatus, info));
    }
    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, mipMapsStatus, info));
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
