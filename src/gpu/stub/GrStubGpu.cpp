/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStubGpu.h"

#include "GrCaps.h"

sk_sp<GrGpu> GrStubGpu::Make(GrContext* context, sk_sp<const GrCaps> caps) {
    return sk_sp<GrGpu>(new GrStubGpu(context, std::move(caps)));
}


GrGpuRTCommandBuffer* GrStubGpu::createCommandBuffer(
                                            GrRenderTarget* rt, GrSurfaceOrigin origin,
                                            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) {
    SkASSERT(0);
    return nullptr;
}

GrGpuTextureCommandBuffer* GrStubGpu::createCommandBuffer(GrTexture* texture,
                                                          GrSurfaceOrigin origin) {
    SkASSERT(0);
    return nullptr;
}

void GrStubGpu::submitCommandBuffer(const GrGpuRTCommandBuffer* cmdBuffer) {
    SkASSERT(0);
}

GrStubGpu::GrStubGpu(GrContext* context, sk_sp<const GrCaps> caps)
        : INHERITED(context) {
    fCaps = caps;
}

sk_sp<GrTexture> GrStubGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                            const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(0);
    return nullptr;
}

GrBuffer* GrStubGpu::onCreateBuffer(size_t sizeInBytes, GrBufferType type,
                                    GrAccessPattern accessPattern, const void*) {
    SkASSERT(0);
    return nullptr;
}

GrStencilAttachment* GrStubGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                       int width,
                                                                       int height) {
    SkASSERT(0);
    return nullptr;
}

GrBackendObject GrStubGpu::createTestingOnlyBackendObject(void* pixels, int w, int h,
                                                          GrPixelConfig config, bool isRT,
                                                          GrMipMapped) {
    SkASSERT(0);
    return reinterpret_cast<GrBackendObject>(nullptr);
}

void GrStubGpu::deleteTestingOnlyBackendObject(GrBackendObject object, bool abandonTexture) {
    SkASSERT(0);
}

GrBackendTexture GrStubGpu::createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                            GrPixelConfig config, bool isRT,
                                                            GrMipMapped) {
    SkASSERT(0);
    return GrBackendTexture(); // invalid
}

bool GrStubGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(0);
    return false;
}

void GrStubGpu::deleteTestingOnlyBackendTexture(GrBackendTexture* tex, bool abandonTexture) {
    SkASSERT(0);
}
