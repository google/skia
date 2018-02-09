/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTGpu.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMesh.h"
#include "GrNXTBuffer.h"
#include "GrNXTCaps.h"
#include "GrNXTRenderTarget.h"
#include "GrNXTGpuCommandBuffer.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrSemaphore.h"
#include "GrTexturePriv.h"

#include "SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

sk_sp<GrGpu> GrNXTGpu::Make(GrBackendContext backendContext, const GrContextOptions& options,
                            GrContext* context) {
    const auto* backend = reinterpret_cast<const GrNXTBackendContext*>(backendContext);
    return Make(sk_ref_sp(backend), options, context);
}

sk_sp<GrGpu> GrNXTGpu::Make(sk_sp<const GrNXTBackendContext> backendContext,
                            const GrContextOptions& options, GrContext* context) {
    if (!backendContext) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrNXTGpu(context, options, backendContext));
}

////////////////////////////////////////////////////////////////////////////////

GrNXTGpu::GrNXTGpu(GrContext* context, const GrContextOptions& options,
                   sk_sp<const GrNXTBackendContext> backendContext)
        : INHERITED(context)
        , fDevice(backendContext->fDevice.Clone())
        , fQueue(backendContext->fQueue.Clone())
        , fResourceProvider(this) {
    fCompiler = new SkSL::Compiler();
    fCaps.reset(new GrNXTCaps(options));
    fResourceProvider.init();
}

GrNXTGpu::~GrNXTGpu() {
    delete fCompiler;
}


void GrNXTGpu::disconnect(DisconnectType type) {
    SkASSERT(!"unimplemented");
}

///////////////////////////////////////////////////////////////////////////////

GrGpuRTCommandBuffer* GrNXTGpu::createCommandBuffer(
            GrRenderTarget* rt, GrSurfaceOrigin origin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrNXTGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrNXTGpu::createCommandBuffer(GrTexture* texture,
                                                         GrSurfaceOrigin origin) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
GrBuffer* GrNXTGpu::onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern accessPattern,
                                   const void* data) {
    GrBuffer* b = new GrNXTBuffer(this, size, type, accessPattern);
    if (data && b) {
        b->updateData(data, size);
    }
    return b;
}

////////////////////////////////////////////////////////////////////////////////
bool GrNXTGpu::onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                                    int width, int height,
                                    GrPixelConfig srcConfig, DrawPreference* drawPreference,
                                    WritePixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrNXTGpu::onWritePixels(GrSurface* surface, GrSurfaceOrigin origin,
                             int left, int top, int width, int height,
                             GrPixelConfig config,
                             const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrNXTGpu::onTransferPixels(GrTexture* texture,
                                int left, int top, int width, int height,
                                GrPixelConfig config, GrBuffer* transferBuffer,
                                size_t bufferOffset, size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrNXTGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                           const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrTexture> GrNXTGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrTexture> GrNXTGpu::onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                          int sampleCnt,
                                                          GrWrapOwnership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrNXTGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget&) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrNXTGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                   int sampleCnt) {
    const GrNXTImageInfo* info = tex.getNXTImageInfo();
    if (!info) {
        return nullptr;
    }
    if (!info->fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin; // Not actually used in the following
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = tex.config();
    desc.fSampleCnt = this->caps()->getSampleCount(sampleCnt, tex.config());

    sk_sp<GrNXTRenderTarget> tgt = GrNXTRenderTarget::MakeWrapped(this, desc, info);
    return tgt;
}

GrStencilAttachment* GrNXTGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                      int width,
                                                                      int height) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

GrBackendTexture GrNXTGpu::createTestingOnlyBackendTexture(void* srcData, int w, int h,
                                                           GrPixelConfig config,
                                                           bool isRenderTarget,
                                                           GrMipMapped mipMapped) {
    SkASSERT(!"unimplemented");
    return GrBackendTexture();
}

bool GrNXTGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(!"unimplemented");
    return false;
}

void GrNXTGpu::deleteTestingOnlyBackendTexture(GrBackendTexture* tex, bool abandon) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpu::onFinishFlush(bool insertedSemaphore) {
}

void GrNXTGpu::clearStencil(GrRenderTarget* target, int clearValue) {
    SkASSERT(!"unimplemented");
}

bool GrNXTGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                             GrSurface* src, GrSurfaceOrigin srcOrigin,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrNXTGpu::onQueryMultisampleSpecs(GrRenderTarget* rt, GrSurfaceOrigin,
                                       const GrStencilSettings&, int* effectiveSampleCnt,
                                       SamplePattern*) {
    SkASSERT(!"unimplemented");
    // TODO: stub.
    *effectiveSampleCnt = rt->numStencilSamples();
}

bool GrNXTGpu::onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                                   int width, int height, size_t rowBytes,
                                   GrPixelConfig readConfig, DrawPreference* drawPreference,
                                   ReadPixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrNXTGpu::onReadPixels(GrSurface* surface, GrSurfaceOrigin origin,
                            int left, int top, int width, int height,
                            GrPixelConfig config,
                            void* buffer,
                            size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

GrFence SK_WARN_UNUSED_RESULT GrNXTGpu::insertFence() {
    SkASSERT(!"unimplemented");
    return GrFence();
}

bool GrNXTGpu::waitFence(GrFence fence, uint64_t timeout) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrNXTGpu::deleteFence(GrFence fence) const {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrNXTGpu::makeSemaphore(bool isOwned) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrSemaphore> GrNXTGpu::onWrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                    GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

void GrNXTGpu::onInsertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpu::onWaitSemaphore(sk_sp<GrSemaphore> semaphore) {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> GrNXTGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(!"unimplemented");
    return nullptr;
}
