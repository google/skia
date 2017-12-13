/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDDLGpu.h"

#include "GrCaps.h"

sk_sp<GrGpu> GrDDLGpu::Make(GrContext* context, sk_sp<const GrCaps> caps) {
    return sk_sp<GrGpu>(new GrDDLGpu(context, std::move(caps)));
}


GrGpuRTCommandBuffer* GrDDLGpu::createCommandBuffer(
                                            GrRenderTarget* rt, GrSurfaceOrigin origin,
                                            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) {
    SkASSERT(0);
    return nullptr;
}

GrGpuTextureCommandBuffer* GrDDLGpu::createCommandBuffer(GrTexture* texture,
                                                         GrSurfaceOrigin origin) {
    SkASSERT(0);
    return nullptr;
}

void GrDDLGpu::submitCommandBuffer(const GrGpuRTCommandBuffer* cmdBuffer) {
    SkASSERT(0);
}

GrDDLGpu::GrDDLGpu(GrContext* context, sk_sp<const GrCaps> caps)
        : INHERITED(context) {
    fCaps = caps;
}

sk_sp<GrTexture> GrDDLGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                           const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(0);
    return nullptr;
}

GrBuffer* GrDDLGpu::onCreateBuffer(size_t sizeInBytes, GrBufferType type,
                                   GrAccessPattern accessPattern, const void*) {
    SkASSERT(0);
    return nullptr;
}

GrStencilAttachment* GrDDLGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                      int width,
                                                                      int height) {
    SkASSERT(0);
    return nullptr;
}

GrBackendTexture GrDDLGpu::createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                           GrPixelConfig config, bool isRT,
                                                           GrMipMapped) {
    SkASSERT(0);
    return GrBackendTexture(); // invalid
}

bool GrDDLGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(0);
    return false;
}

void GrDDLGpu::deleteTestingOnlyBackendTexture(GrBackendTexture* tex, bool abandonTexture) {
    SkASSERT(0);
}
