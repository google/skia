/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordingContext.h"

#include "GrMemoryPool.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceContext.h"
#include "SkGr.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)

////////////////////////////////////////////////////////////////////////////////
GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t uniqueID)
        : INHERITED(backend, options, uniqueID) {
}

GrRecordingContext::~GrRecordingContext() { }

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

sk_sp<GrSurfaceContext> GrRecordingContext::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER

    if (proxy->asRenderTargetProxy()) {
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                               std::move(colorSpace), props);
    } else {
        SkASSERT(proxy->asTextureProxy());
        SkASSERT(!props);
        return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
    }
}

sk_sp<GrSurfaceContext> GrRecordingContext::makeDeferredSurfaceContext(
                                                                  const GrBackendFormat& format,
                                                                  const GrSurfaceDesc& dstDesc,
                                                                  GrSurfaceOrigin origin,
                                                                  GrMipMapped mipMapped,
                                                                  SkBackingFit fit,
                                                                  SkBudgeted isDstBudgeted,
                                                                  sk_sp<SkColorSpace> colorSpace,
                                                                  const SkSurfaceProps* props) {
    sk_sp<GrTextureProxy> proxy;
    if (GrMipMapped::kNo == mipMapped) {
        proxy = this->proxyProvider()->createProxy(format, dstDesc, origin, fit, isDstBudgeted);
    } else {
        SkASSERT(SkBackingFit::kExact == fit);
        proxy = this->proxyProvider()->createMipMapProxy(format, dstDesc, origin, isDstBudgeted);
    }
    if (!proxy) {
        return nullptr;
    }

    sk_sp<GrSurfaceContext> sContext = this->makeWrappedSurfaceContext(std::move(proxy),
                                                                       std::move(colorSpace),
                                                                       props);
    if (sContext && sContext->asRenderTargetContext()) {
        sContext->asRenderTargetContext()->discard();
    }

    return sContext;
}

static inline GrPixelConfig GrPixelConfigFallback(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kSBGRA_8888_GrPixelConfig:
            return kSRGBA_8888_GrPixelConfig;
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
            return kRGBA_half_GrPixelConfig;
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
            return kRGB_888_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

sk_sp<GrRenderTargetContext> GrRecordingContext::makeDeferredRenderTargetContextWithFallback(
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
    GrBackendFormat localFormat = format;
    SkASSERT(sampleCnt > 0);
    if (0 == this->priv().caps()->getRenderTargetSampleCount(sampleCnt, config)) {
        config = GrPixelConfigFallback(config);
        // TODO: First we should be checking the getRenderTargetSampleCount from the GrBackendFormat
        // and not GrPixelConfig. Besides that, we should implement the fallback in the caps, but
        // for now we just convert the fallback pixel config to an SkColorType and then get the
        // GrBackendFormat from that.
        SkColorType colorType;
        if (!GrPixelConfigToColorType(config, &colorType)) {
            return nullptr;
        }
        localFormat = this->priv().caps()->getBackendFormatFromColorType(colorType);
    }

    return this->makeDeferredRenderTargetContext(localFormat, fit, width, height, config,
                                                 std::move(colorSpace), sampleCnt, mipMapped,
                                                 origin, surfaceProps, budgeted);
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
    if (this->abandoned1()) {
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

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fContext->fDrawingManager->makeRenderTargetContext(std::move(rtp),
                                                           std::move(colorSpace),
                                                           surfaceProps));
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrOpMemoryPool> GrRecordingContextPriv::refOpMemoryPool() {
    return fContext->refOpMemoryPool();
}

sk_sp<GrSurfaceContext> GrRecordingContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                          sk_sp<SkColorSpace> cs,
                                                                          const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), std::move(cs), props);
}

sk_sp<GrSurfaceContext> GrRecordingContextPriv::makeDeferredSurfaceContext(
                                                                    const GrBackendFormat& format,
                                                                    const GrSurfaceDesc& desc,
                                                                    GrSurfaceOrigin origin,
                                                                    GrMipMapped mipMapped,
                                                                    SkBackingFit fit,
                                                                    SkBudgeted budgeted,
                                                                    sk_sp<SkColorSpace> colorSpace,
                                                                    const SkSurfaceProps* props) {
    return fContext->makeDeferredSurfaceContext(format, desc, origin, mipMapped, fit, budgeted,
                                                std::move(colorSpace), props);
}
