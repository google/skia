/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/text/GrTextBlobCache.h"

#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)

GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t contextID)
        : INHERITED(backend, options, contextID)
        , fAuditTrail(new GrAuditTrail()) {
}

GrRecordingContext::~GrRecordingContext() { }

/**
 * TODO: move textblob draw calls below context (see comment below)
 */
static void textblobcache_overbudget_CB(void* data) {
    SkASSERT(data);
    GrRecordingContext* context = reinterpret_cast<GrRecordingContext*>(data);

    GrContext* direct = context->priv().asDirectContext();
    if (!direct) {
        return;
    }

    // TextBlobs are drawn at the SkGpuDevice level, therefore they cannot rely on
    // GrRenderTargetContext to perform a necessary flush.  The solution is to move drawText calls
    // to below the GrContext level, but this is not trivial because they call drawPath on
    // SkGpuDevice.
    direct->flush();
}

bool GrRecordingContext::init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> cache) {

    if (!INHERITED::init(std::move(caps), std::move(cache))) {
        return false;
    }

    fStrikeCache.reset(new GrStrikeCache(this->caps(),
                                        this->options().fGlyphCacheTextureMaximumBytes));

    fTextBlobCache.reset(new GrTextBlobCache(textblobcache_overbudget_CB, this,
                                             this->contextID()));

    return true;
}

void GrRecordingContext::setupDrawingManager(bool sortOpsTasks, bool reduceOpsTaskSplitting) {
    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = this->options().fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = this->options().fGpuPathRenderers;
#endif
    // FIXME: Once this is removed from Chrome and Android, rename to fEnable"".
    if (!this->options().fDisableCoverageCountingPaths) {
        prcOptions.fGpuPathRenderers |= GpuPathRenderers::kCoverageCounting;
    }
    if (this->options().fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    if (!this->proxyProvider()->renderingDirectly()) {
        // DDL TODO: remove this crippling of the path renderer chain
        // Disable the small path renderer bc of the proxies in the atlas. They need to be
        // unified when the opsTasks are added back to the destination drawing manager.
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    GrTextContext::Options textContextOptions;
    textContextOptions.fMaxDistanceFieldFontSize = this->options().fGlyphsAsPathsFontSize;
    textContextOptions.fMinDistanceFieldFontSize = this->options().fMinDistanceFieldFontSize;
    textContextOptions.fDistanceFieldVerticesAlwaysHaveW = false;
#if SK_SUPPORT_ATLAS_TEXT
    if (GrContextOptions::Enable::kYes == this->options().fDistanceFieldGlyphVerticesAlwaysHaveW) {
        textContextOptions.fDistanceFieldVerticesAlwaysHaveW = true;
    }
#endif

    fDrawingManager.reset(new GrDrawingManager(this,
                                               prcOptions,
                                               textContextOptions,
                                               sortOpsTasks,
                                               reduceOpsTaskSplitting));
}

void GrRecordingContext::abandonContext() {
    INHERITED::abandonContext();

    fStrikeCache->freeAll();
    fTextBlobCache->freeAll();
}

GrDrawingManager* GrRecordingContext::drawingManager() {
    return fDrawingManager.get();
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

GrTextBlobCache* GrRecordingContext::getTextBlobCache() {
    return fTextBlobCache.get();
}

const GrTextBlobCache* GrRecordingContext::getTextBlobCache() const {
    return fTextBlobCache.get();
}

void GrRecordingContext::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->drawingManager()->addOnFlushCallbackObject(onFlushCBObject);
}

std::unique_ptr<GrSurfaceContext> GrRecordingContext::makeWrappedSurfaceContext(
        sk_sp<GrSurfaceProxy> proxy,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV

    SkASSERT(proxy);

    if (proxy->asRenderTargetProxy()) {
        SkASSERT(kPremul_SkAlphaType == alphaType || kOpaque_SkAlphaType == alphaType);
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy), colorType,
                                                               std::move(colorSpace), props);
    } else {
        SkASSERT(proxy->asTextureProxy());
        SkASSERT(!props);
        return this->drawingManager()->makeTextureContext(std::move(proxy), colorType, alphaType,
                                                          std::move(colorSpace));
    }
}

std::unique_ptr<GrTextureContext> GrRecordingContext::makeDeferredTextureContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    auto format = this->caps()->getDefaultBackendFormat(colorType, GrRenderable::kNo);
    if (!format.isValid()) {
        return nullptr;
    }
    auto config = this->caps()->getConfigFromBackendFormat(format, colorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    sk_sp<GrTextureProxy> texture = this->proxyProvider()->createProxy(
            format, desc, GrRenderable::kNo, 1, origin, mipMapped, fit, budgeted, isProtected);
    if (!texture) {
        return nullptr;
    }

    auto drawingManager = this->drawingManager();

    return drawingManager->makeTextureContext(std::move(texture), colorType, alphaType,
                                              std::move(colorSpace));
}

std::unique_ptr<GrRenderTargetContext> GrRecordingContext::makeDeferredRenderTargetContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    SkASSERT(sampleCnt > 0);
    if (this->abandoned()) {
        return nullptr;
    }

    auto format = this->caps()->getDefaultBackendFormat(colorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return nullptr;
    }
    auto config = this->caps()->getConfigFromBackendFormat(format, colorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    sk_sp<GrTextureProxy> rtp =
            this->proxyProvider()->createProxy(format, desc, GrRenderable::kYes, sampleCnt, origin,
                                               mipMapped, fit, budgeted, isProtected);
    if (!rtp) {
        return nullptr;
    }

    auto drawingManager = this->drawingManager();

    auto renderTargetContext = drawingManager->makeRenderTargetContext(
            std::move(rtp), colorType, std::move(colorSpace), surfaceProps);
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

static inline GrColorType color_type_fallback(GrColorType ct) {
    switch (ct) {
        // kRGBA_8888 is our default fallback for many color types that may not have renderable
        // backend formats.
        case GrColorType::kAlpha_8:
        case GrColorType::kBGR_565:
        case GrColorType::kABGR_4444:
        case GrColorType::kBGRA_8888:
        case GrColorType::kRGBA_1010102:
        case GrColorType::kRGBA_F16:
        case GrColorType::kRGBA_F16_Clamped:
            return GrColorType::kRGBA_8888;
        case GrColorType::kAlpha_F16:
            return GrColorType::kRGBA_F16;
        case GrColorType::kGray_8:
            return GrColorType::kRGB_888x;
        default:
            return GrColorType::kUnknown;
    }
}

std::unique_ptr<GrRenderTargetContext>
GrRecordingContext::makeDeferredRenderTargetContextWithFallback(SkBackingFit fit,
                                                                int width,
                                                                int height,
                                                                GrColorType colorType,
                                                                sk_sp<SkColorSpace> colorSpace,
                                                                int sampleCnt,
                                                                GrMipMapped mipMapped,
                                                                GrSurfaceOrigin origin,
                                                                const SkSurfaceProps* surfaceProps,
                                                                SkBudgeted budgeted,
                                                                GrProtected isProtected) {
    SkASSERT(sampleCnt > 0);
    std::unique_ptr<GrRenderTargetContext> rtc;
    do {
        rtc = this->makeDeferredRenderTargetContext(fit, width, height, colorType, colorSpace,
                                                    sampleCnt, mipMapped, origin, surfaceProps,
                                                    budgeted, isProtected);
        colorType = color_type_fallback(colorType);
    } while (!rtc && colorType != GrColorType::kUnknown);
    return rtc;
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

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

std::unique_ptr<GrSurfaceContext> GrRecordingContextPriv::makeWrappedSurfaceContext(
        sk_sp<GrSurfaceProxy> proxy,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), colorType, alphaType,
                                               std::move(colorSpace), props);
}

std::unique_ptr<GrTextureContext> GrRecordingContextPriv::makeDeferredTextureContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredTextureContext(fit, width, height, colorType, alphaType,
                                                std::move(colorSpace), mipMapped, origin, budgeted,
                                                isProtected);
}

std::unique_ptr<GrRenderTargetContext> GrRecordingContextPriv::makeDeferredRenderTargetContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContext(fit, width, height, colorType,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted, isProtected);
}

std::unique_ptr<GrRenderTargetContext>
GrRecordingContextPriv::makeDeferredRenderTargetContextWithFallback(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContextWithFallback(fit,
                                                                 width,
                                                                 height,
                                                                 colorType,
                                                                 std::move(colorSpace),
                                                                 sampleCnt,
                                                                 mipMapped,
                                                                 origin,
                                                                 surfaceProps,
                                                                 budgeted,
                                                                 isProtected);
}

GrContext* GrRecordingContextPriv::backdoor() {
    return (GrContext*) fContext;
}

