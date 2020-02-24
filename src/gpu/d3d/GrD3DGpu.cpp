/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DGpu.h"

sk_sp<GrGpu> GrD3DGpu::Make(const GrD3DBackendContext& backendContext,
                            const GrContextOptions& contextOptions, GrContext* context) {
    return sk_sp<GrGpu>(new GrD3DGpu(context, contextOptions, backendContext));
}

GrD3DGpu::GrD3DGpu(GrContext* context, const GrContextOptions& contextOptions,
                   const GrD3DBackendContext& backendContext)
        : INHERITED(context)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fProtectedContext(backendContext.fProtectedContext) {
    fCaps.reset(new GrD3DCaps(contextOptions));
}

GrOpsRenderPass* GrD3DGpu::getOpsRenderPass(
    GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
    const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
    const GrOpsRenderPass::StencilLoadAndStoreInfo&,
    const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    // TODO
    return nullptr;
}

void GrD3DGpu::submit(GrOpsRenderPass* renderPass) {
    // TODO: actually submit something here
    delete renderPass;
}

void GrD3DGpu::querySampleLocations(GrRenderTarget* rt, SkTArray<SkPoint>* sampleLocations) {
    // TODO
}

sk_sp<GrTexture> GrD3DGpu::onCreateTexture(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           GrRenderable renderable,
                                           int renderTargetSampleCnt,
                                           SkBudgeted budgeted,
                                           GrProtected isProtected,
                                           int mipLevelCount,
                                           uint32_t levelClearMask) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onCreateCompressedTexture(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     SkBudgeted budgeted,
                                                     GrMipMapped mipMapped,
                                                     GrProtected isProtected,
                                                     const void* data, size_t dataSize) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapBackendTexture(const GrBackendTexture& tex, GrColorType colorType,
                                                GrWrapOwnership ownership,
                                                GrWrapCacheable wrapType, GrIOType ioType) {
    // TODO
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable wrapType) {
    return nullptr;
}

sk_sp<GrTexture> GrD3DGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                          int sampleCnt,
                                                          GrColorType colorType,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable cacheable) {
    // TODO
    return nullptr;
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                          GrColorType colorType) {
    // TODO
    return nullptr;
}

sk_sp<GrRenderTarget> GrD3DGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt,
                                                                    GrColorType colorType) {
    // TODO
    return nullptr;
}

sk_sp<GrGpuBuffer> GrD3DGpu::onCreateBuffer(size_t sizeInBytes, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void*) {
    // TODO
    return nullptr;
}

GrStencilAttachment* GrD3DGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    // TODO
    return nullptr;
}

GrBackendTexture GrD3DGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrRenderable,
                                                   GrMipMapped mipMapped,
                                                   GrProtected,
                                                   const BackendTextureData*) {
    // TODO
    return GrBackendTexture();
}

GrBackendTexture GrD3DGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                             const GrBackendFormat& format,
                                                             GrMipMapped mipMapped,
                                                             GrProtected,
                                                             const BackendTextureData*) {
    // TODO
    return GrBackendTexture();
}

void GrD3DGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    // TODO
}

bool GrD3DGpu::compile(const GrProgramDesc&, const GrProgramInfo&) {
    return false;
}

#if GR_TEST_UTILS
bool GrD3DGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    // TODO
    return false;
}

GrBackendRenderTarget GrD3DGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                      GrColorType colorType) {
    // TODO
    return GrBackendRenderTarget();
}

void GrD3DGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {}
#endif
