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

void GrRecordingContext::setupDrawingManager(bool sortOpLists, bool reduceOpListSplitting) {
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
        // unified when the opLists are added back to the destination drawing manager.
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
                                               sortOpLists,
                                               reduceOpListSplitting));
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

sk_sp<GrSurfaceContext> GrRecordingContext::makeWrappedSurfaceContext(
        sk_sp<GrSurfaceProxy> proxy,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV

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

sk_sp<GrSurfaceContext> GrRecordingContext::makeDeferredSurfaceContext(
        const GrBackendFormat& format,
        const GrSurfaceDesc& dstDesc,
        GrSurfaceOrigin origin,
        GrMipMapped mipMapped,
        SkBackingFit fit,
        SkBudgeted isDstBudgeted,
        GrColorType colorType,
        SkAlphaType alphaType,
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
                                                                       colorType,
                                                                       alphaType,
                                                                       std::move(colorSpace),
                                                                       props);
    if (sContext && sContext->asRenderTargetContext()) {
        sContext->asRenderTargetContext()->discard();
    }

    return sContext;
}

sk_sp<GrRenderTargetContext> GrRecordingContext::makeDeferredRenderTargetContext(
        const GrBackendFormat& format,
        SkBackingFit fit,
        int width,
        int height,
        GrPixelConfig config,
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

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fIsProtected = isProtected;
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

    auto drawingManager = this->drawingManager();

    sk_sp<GrRenderTargetContext> renderTargetContext = drawingManager->makeRenderTargetContext(
            std::move(rtp), colorType, std::move(colorSpace), surfaceProps);
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

static inline bool color_type_and_config_fallback(GrColorType* ct, GrPixelConfig* config) {
    switch (*ct) {
        case GrColorType::kAlpha_8:
            if (*config != kAlpha_8_GrPixelConfig && *config != kAlpha_8_as_Red_GrPixelConfig &&
                *config != kAlpha_8_as_Alpha_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kBGR_565:
            if (*config != kRGB_565_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kABGR_4444:
            if (*config != kRGBA_4444_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kBGRA_8888:
            if (*config != kBGRA_8888_GrPixelConfig && *config != kSBGRA_8888_GrPixelConfig) {
                return false;
            }
            *config = (*config == kSBGRA_8888_GrPixelConfig) ? kSRGBA_8888_GrPixelConfig
                                                             : kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kRGBA_1010102:
            if (*config != kRGBA_1010102_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kRGBA_F16:
            if (*config != kRGBA_half_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kRGBA_F16_Clamped:
            if (*config != kRGBA_half_Clamped_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_8888_GrPixelConfig;
            *ct = GrColorType::kRGBA_8888;
            return true;
        case GrColorType::kAlpha_F16:
            if (*config != kAlpha_half_GrPixelConfig &&
                *config != kAlpha_half_as_Red_GrPixelConfig) {
                return false;
            }
            *config = kRGBA_half_GrPixelConfig;
            *ct = GrColorType::kRGBA_F16;
            return true;
        case GrColorType::kGray_8:
            if (*config != kGray_8_GrPixelConfig && *config != kGray_8_as_Red_GrPixelConfig &&
                *config != kGray_8_as_Lum_GrPixelConfig) {
                return false;
            }
            *config = kRGB_888_GrPixelConfig;
            *ct = GrColorType::kRGB_888x;
            return true;
        default:
            return false;
    }
}

sk_sp<GrRenderTargetContext> GrRecordingContext::makeDeferredRenderTargetContextWithFallback(
        const GrBackendFormat& format,
        SkBackingFit fit,
        int width,
        int height,
        GrPixelConfig config,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    GrBackendFormat localFormat = format;
    SkASSERT(sampleCnt > 0);
    if (0 == this->caps()->getRenderTargetSampleCount(sampleCnt, config)) {
        // TODO: Make the fallback part of GrCaps?
        if (!color_type_and_config_fallback(&colorType, &config)) {
            return nullptr;
        }
        // Figure out what the new backend format should be for the new color type.
        auto srgb = GrPixelConfigIsSRGBEncoded(config);
        localFormat = this->caps()->getBackendFormatFromGrColorType(colorType, srgb);
    }

    return this->makeDeferredRenderTargetContext(localFormat, fit, width, height, config, colorType,
                                                 std::move(colorSpace), sampleCnt, mipMapped,
                                                 origin, surfaceProps, budgeted, isProtected);
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

sk_sp<GrSurfaceContext> GrRecordingContextPriv::makeWrappedSurfaceContext(
        sk_sp<GrSurfaceProxy> proxy,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), colorType, alphaType,
                                               std::move(colorSpace), props);
}

sk_sp<GrSurfaceContext> GrRecordingContextPriv::makeDeferredSurfaceContext(
        const GrBackendFormat& format,
        const GrSurfaceDesc& dstDesc,
        GrSurfaceOrigin origin,
        GrMipMapped mipMapped,
        SkBackingFit fit,
        SkBudgeted isDstBudgeted,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    return fContext->makeDeferredSurfaceContext(format, dstDesc, origin, mipMapped, fit,
                                                isDstBudgeted, colorType, alphaType,
                                                std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrRecordingContextPriv::makeDeferredRenderTargetContext(
        const GrBackendFormat& format,
        SkBackingFit fit,
        int width,
        int height,
        GrPixelConfig config,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContext(format, fit, width, height, config, colorType,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted, isProtected);
}

sk_sp<GrRenderTargetContext> GrRecordingContextPriv::makeDeferredRenderTargetContextWithFallback(
        const GrBackendFormat& format,
        SkBackingFit fit,
        int width,
        int height,
        GrPixelConfig config,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContextWithFallback(format,
                                                                 fit,
                                                                 width,
                                                                 height,
                                                                 config,
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
