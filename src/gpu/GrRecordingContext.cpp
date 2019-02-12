/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordingContext.h"

#include "GrCaps.h"
#include "GrDrawingManager.h"
#include "GrMemoryPool.h"
#include "GrProxyProvider.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrSkSLFPFactoryCache.h"

GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t contextID)
        : INHERITED(backend, options, contextID) {
}

GrRecordingContext::~GrRecordingContext() { }

void GrRecordingContext::abandonContext() {
    INHERITED::abandonContext();
}

sk_sp<GrOpMemoryPool> GrRecordingContext::refOpMemoryPool() {
    if (!fOpMemoryPool) {
        // DDL TODO: should the size of the memory pool be decreased in DDL mode? CPU-side memory
        // consumed in DDL mode vs. normal mode for a single skp might be a good metric of wasted
        // memory.
        fOpMemoryPool = sk_sp<GrOpMemoryPool>(new GrOpMemoryPool(16384, 16384));
    }

    SkASSERT(fOpMemoryPool);
    return fOpMemoryPool;
}

GrOpMemoryPool* GrRecordingContext::opMemoryPool() {
    return this->refOpMemoryPool().get();
}

sk_sp<GrRenderTargetContext> GrRecordingContext::makeDeferredRenderTargetContext(
                                                        const GrBackendFormat& format,
                                                        SkBackingFit fit,
                                                        int width, int height,
                                                        GrPixelConfig config,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        int sampleCnt,
                                                        GrMipMapped mipMapped,
                                                        GrSurfaceOrigin origin,
                                                        const SkSurfaceProps* surfaceProps,
                                                        SkBudgeted budgeted) {
    SkASSERT(sampleCnt > 0);
    if (this->abandoned()) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrTextureProxy> rtp;
    if (GrMipMapped::kNo == mipMapped) {
        rtp = this->proxyProvider()->createProxy(format, desc, origin, fit, budgeted);
    } else {
        rtp = this->proxyProvider()->createMipMapProxy(format, desc, origin, budgeted);
    }
    if (!rtp) {
        return nullptr;
    }

    // CONTEXT TODO: move GrDrawingManager to GrRecordingContext for real
    auto drawingManager = this->drawingManager();

    sk_sp<GrRenderTargetContext> renderTargetContext =
        drawingManager->makeRenderTargetContext(std::move(rtp), std::move(colorSpace),
                                                surfaceProps);
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrRecordingContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrRecordingContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

sk_sp<GrOpMemoryPool> GrRecordingContextPriv::refOpMemoryPool() {
    return fContext->refOpMemoryPool();
}

sk_sp<GrRenderTargetContext> GrRecordingContextPriv::makeDeferredRenderTargetContext(
                                                    const GrBackendFormat& format,
                                                    SkBackingFit fit,
                                                    int width, int height,
                                                    GrPixelConfig config,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    int sampleCnt,
                                                    GrMipMapped mipMapped,
                                                    GrSurfaceOrigin origin,
                                                    const SkSurfaceProps* surfaceProps,
                                                    SkBudgeted budgeted) {
    return fContext->makeDeferredRenderTargetContext(format, fit, width, height, config,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted);
}
