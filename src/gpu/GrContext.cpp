/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrClip.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfaceContext.h"
#include "GrSurfacePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureContext.h"
#include "SkConvertPixels.h"
#include "SkGr.h"
#include "SkUnPreMultiplyPriv.h"
#include "effects/GrConfigConversionEffect.h"
#include "text/GrTextBlobCache.h"

#define ASSERT_OWNED_PROXY(P) \
SkASSERT(!(P) || !((P)->priv().peekTexture()) || (P)->priv().peekTexture()->getContext() == this)
#define ASSERT_OWNED_PROXY_PRIV(P) \
SkASSERT(!(P) || !((P)->priv().peekTexture()) || (P)->priv().peekTexture()->getContext() == fContext)

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fSingleOwner);)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fContext->fSingleOwner);)
#define RETURN_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return; }
#define RETURN_IF_ABANDONED_PRIV if (fContext->fDrawingManager->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_FALSE_IF_ABANDONED_PRIV if (fContext->fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return nullptr; }

////////////////////////////////////////////////////////////////////////////////

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext) {
    GrContextOptions defaultOptions;
    return Create(backend, backendContext, defaultOptions);
}

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext,
                             const GrContextOptions& options) {
    GrContext* context = new GrContext;

    if (context->init(backend, backendContext, options)) {
        return context;
    } else {
        context->unref();
        return nullptr;
    }
}

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

GrContext::GrContext() : fUniqueID(next_id()) {
    fGpu = nullptr;
    fCaps = nullptr;
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
    fAtlasGlyphCache = nullptr;
}

bool GrContext::init(GrBackend backend, GrBackendContext backendContext,
                     const GrContextOptions& options) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!fGpu);

    fGpu = GrGpu::Create(backend, backendContext, options, this);
    if (!fGpu) {
        return false;
    }
    this->initCommon(options);
    return true;
}

void GrContext::initCommon(const GrContextOptions& options) {
    ASSERT_SINGLE_OWNER

    fCaps = SkRef(fGpu->caps());
    fResourceCache = new GrResourceCache(fCaps);
    fResourceProvider = new GrResourceProvider(fGpu, fResourceCache, &fSingleOwner);

    fDisableGpuYUVConversion = options.fDisableGpuYUVConversion;
    fDidTestPMConversions = false;

    GrRenderTargetOpList::Options rtOpListOptions;
    rtOpListOptions.fMaxOpCombineLookback = options.fMaxOpCombineLookback;
    rtOpListOptions.fMaxOpCombineLookahead = options.fMaxOpCombineLookahead;
    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = options.fAllowPathMaskCaching;
    prcOptions.fGpuPathRenderers = options.fGpuPathRenderers;
    fDrawingManager.reset(new GrDrawingManager(this, rtOpListOptions, prcOptions,
                                               options.fImmediateMode, &fSingleOwner));

    fAtlasGlyphCache = new GrAtlasGlyphCache(this);

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB, this));
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    if (!fGpu) {
        SkASSERT(!fCaps);
        return;
    }

    this->flush();

    fDrawingManager->cleanup();

    for (int i = 0; i < fCleanUpData.count(); ++i) {
        (*fCleanUpData[i].fFunc)(this, fCleanUpData[i].fInfo);
    }

    delete fResourceProvider;
    delete fResourceCache;
    delete fAtlasGlyphCache;

    fGpu->unref();
    fCaps->unref();
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    if (!fThreadSafeProxy) {
        fThreadSafeProxy.reset(new GrContextThreadSafeProxy(sk_ref_sp(fCaps), this->uniqueID()));
    }
    return fThreadSafeProxy;
}

void GrContext::abandonContext() {
    ASSERT_SINGLE_OWNER

    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fAtlasGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::releaseResourcesAndAbandonContext() {
    ASSERT_SINGLE_OWNER

    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);

    fAtlasGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    this->flush();

    fAtlasGlyphCache->freeAll();

    fDrawingManager->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
}

void GrContext::purgeResourcesNotUsedInMs(std::chrono::milliseconds ms) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeResourcesNotUsedSince(GrStdSteadyClock::now() - ms);
}

void GrContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
    ASSERT_SINGLE_OWNER

    if (resourceCount) {
        *resourceCount = fResourceCache->getBudgetedResourceCount();
    }
    if (resourceBytes) {
        *resourceBytes = fResourceCache->getBudgetedResourceBytes();
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::TextBlobCacheOverBudgetCB(void* data) {
    SkASSERT(data);
    // TextBlobs are drawn at the SkGpuDevice level, therefore they cannot rely on
    // GrRenderTargetContext to perform a necessary flush.  The solution is to move drawText calls
    // to below the GrContext level, but this is not trivial because they call drawPath on
    // SkGpuDevice.
    GrContext* context = reinterpret_cast<GrContext*>(data);
    context->flush();
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED

    fDrawingManager->flush(nullptr);
}

void GrContextPriv::flush(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    ASSERT_OWNED_PROXY_PRIV(proxy);

    fContext->fDrawingManager->flush(proxy);
}

bool sw_convert_to_premul(GrPixelConfig srcConfig, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkColorType colorType;
    if (!GrPixelConfigToColorType(srcConfig, &colorType) ||
        4 != SkColorTypeBytesPerPixel(colorType))
    {
        return false;
    }

    for (int y = 0; y < height; y++) {
        SkOpts::RGBA_to_rgbA((uint32_t*) outPixels, inPixels, width);
        outPixels = SkTAddOffset<void>(outPixels, outRowBytes);
        inPixels = SkTAddOffset<const void>(inPixels, inRowBytes);
    }

    return true;
}

static bool valid_unpremul_config(GrPixelConfig config) {
    return GrPixelConfigIs8888Unorm(config) || kRGBA_half_GrPixelConfig == config;
}

bool GrContextPriv::writeSurfacePixels(GrSurfaceProxy* dstProxy, SkColorSpace* dstColorSpace,
                                       int left, int top, int width, int height,
                                       GrPixelConfig srcConfig, SkColorSpace* srcColorSpace,
                                       const void* buffer, size_t rowBytes,
                                       uint32_t pixelOpsFlags) {
    // TODO: Color space conversion

    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    ASSERT_OWNED_PROXY_PRIV(dstProxy);
    SkASSERT(dstProxy);
    GR_AUDIT_TRAIL_AUTO_FRAME(&fContext->fAuditTrail, "GrContextPriv::writeSurfacePixels");

    GrSurface* surface = dstProxy->instantiate(fContext->resourceProvider());
    if (!surface) {
        return false;
    }

    fContext->testPMConversionsIfNecessary(pixelOpsFlags);

    // Trim the params here so that if we wind up making a temporary surface it can be as small as
    // necessary and because GrGpu::getWritePixelsInfo requires it.
    if (!GrSurfacePriv::AdjustWritePixelParams(surface->width(), surface->height(),
                                               GrBytesPerPixel(srcConfig), &left, &top, &width,
                                               &height, &buffer, &rowBytes)) {
        return false;
    }

    bool applyPremulToSrc = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);
    if (applyPremulToSrc && !valid_unpremul_config(srcConfig)) {
        return false;
    }
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(surface->config()) != GrPixelConfigIsSint(srcConfig)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
    // Don't prefer to draw for the conversion (and thereby access a texture from the cache) when
    // we've already determined that there isn't a roundtrip preserving conversion processor pair.
    if (applyPremulToSrc && fContext->validPMUPMConversionExists(srcConfig)) {
        drawPreference = GrGpu::kCallerPrefersDraw_DrawPreference;
    }

    GrGpu::WritePixelTempDrawInfo tempDrawInfo;
    if (!fContext->fGpu->getWritePixelsInfo(surface, width, height, srcConfig,
                                            &drawPreference, &tempDrawInfo)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) && surface->surfacePriv().hasPendingIO()) {
        this->flush(nullptr); // MDB TODO: tighten this
    }

    sk_sp<GrTextureProxy> tempProxy;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        tempProxy = GrSurfaceProxy::MakeDeferred(fContext->resourceProvider(),
                                                 tempDrawInfo.fTempSurfaceDesc,
                                                 SkBackingFit::kApprox,
                                                 SkBudgeted::kYes);
        if (!tempProxy && GrGpu::kRequireDraw_DrawPreference == drawPreference) {
            return false;
        }
    }

    // temp buffer for doing sw premul conversion, if needed.
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);
    if (tempProxy) {
        sk_sp<GrFragmentProcessor> texFP = GrSimpleTextureEffect::Make(
                fContext->resourceProvider(), tempProxy, nullptr, SkMatrix::I());
        sk_sp<GrFragmentProcessor> fp;
        if (applyPremulToSrc) {
            fp = fContext->createUPMToPMEffect(texFP, tempProxy->config());
            if (fp) {
                // We no longer need to do this on CPU before the upload.
                applyPremulToSrc = false;
            } else if (GrGpu::kCallerPrefersDraw_DrawPreference == drawPreference) {
                // We only wanted to do the draw to perform the premul so don't bother.
                tempProxy.reset(nullptr);
            }
        }
        if (tempProxy) {
            if (!fp) {
                fp = std::move(texFP);
            }
            fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), tempDrawInfo.fSwizzle);
            SkASSERT(fp);

            if (tempProxy->priv().hasPendingIO()) {
                this->flush(tempProxy.get());
            }
            GrTexture* texture = tempProxy->instantiate(fContext->resourceProvider());
            if (!texture) {
                return false;
            }
            if (applyPremulToSrc) {
                size_t tmpRowBytes = 4 * width;
                tmpPixels.reset(width * height);
                if (!sw_convert_to_premul(srcConfig, width, height, rowBytes, buffer, tmpRowBytes,
                                          tmpPixels.get())) {
                    return false;
                }
                rowBytes = tmpRowBytes;
                buffer = tmpPixels.get();
                applyPremulToSrc = false;
            }
            if (!fContext->fGpu->writePixels(texture, 0, 0, width, height,
                                             tempDrawInfo.fWriteConfig, buffer,
                                             rowBytes)) {
                return false;
            }
            SkMatrix matrix;
            matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            // TODO: Need to decide the semantics of this function for color spaces. Do we support
            // conversion from a passed-in color space? For now, specifying nullptr means that this
            // path will do no conversion, so it will match the behavior of the non-draw path.
            GrRenderTarget* renderTarget = surface->asRenderTarget();
            SkASSERT(renderTarget);
            sk_sp<GrRenderTargetContext> renderTargetContext(
                this->makeWrappedRenderTargetContext(sk_ref_sp(renderTarget), nullptr));
            if (!renderTargetContext) {
                return false;
            }
            GrPaint paint;
            paint.addColorFragmentProcessor(std::move(fp));
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            paint.setAllowSRGBInputs(true);
            SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
            renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix, rect,
                                          nullptr);

            if (kFlushWrites_PixelOp & pixelOpsFlags) {
                this->flushSurfaceWrites(renderTargetContext->asRenderTargetProxy());
            }
        }
    }
    if (!tempProxy) {
        if (applyPremulToSrc) {
            size_t tmpRowBytes = 4 * width;
            tmpPixels.reset(width * height);
            if (!sw_convert_to_premul(srcConfig, width, height, rowBytes, buffer, tmpRowBytes,
                                      tmpPixels.get())) {
                return false;
            }
            rowBytes = tmpRowBytes;
            buffer = tmpPixels.get();
            applyPremulToSrc = false;
        }
        return fContext->fGpu->writePixels(surface, left, top, width, height, srcConfig,
                                           buffer, rowBytes);
    }
    return true;
}

bool GrContextPriv::readSurfacePixels(GrSurfaceProxy* srcProxy, SkColorSpace* srcColorSpace,
                                      int left, int top, int width, int height,
                                      GrPixelConfig dstConfig, SkColorSpace* dstColorSpace,
                                      void* buffer, size_t rowBytes, uint32_t flags) {
    // TODO: Color space conversion

    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    ASSERT_OWNED_PROXY_PRIV(srcProxy);
    SkASSERT(srcProxy);
    GR_AUDIT_TRAIL_AUTO_FRAME(&fContext->fAuditTrail, "GrContextPriv::readSurfacePixels");

    // MDB TODO: delay this instantiation until later in the method
    GrSurface* src = srcProxy->instantiate(fContext->resourceProvider());
    if (!src) {
        return false;
    }

    fContext->testPMConversionsIfNecessary(flags);

    // Adjust the params so that if we wind up using an intermediate surface we've already done
    // all the trimming and the temporary can be the min size required.
    if (!GrSurfacePriv::AdjustReadPixelParams(src->width(), src->height(),
                                              GrBytesPerPixel(dstConfig), &left,
                                              &top, &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & flags) && src->surfacePriv().hasPendingWrite()) {
        this->flush(nullptr); // MDB TODO: tighten this
    }

    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & flags);
    if (unpremul && !valid_unpremul_config(dstConfig)) {
        // The unpremul flag is only allowed for 8888 and F16 configs.
        return false;
    }
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(src->config()) != GrPixelConfigIsSint(dstConfig)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
    // Don't prefer to draw for the conversion (and thereby access a texture from the cache) when
    // we've already determined that there isn't a roundtrip preserving conversion processor pair.
    if (unpremul && fContext->validPMUPMConversionExists(src->config())) {
        drawPreference = GrGpu::kCallerPrefersDraw_DrawPreference;
    }

    GrGpu::ReadPixelTempDrawInfo tempDrawInfo;
    if (!fContext->fGpu->getReadPixelsInfo(src, width, height, rowBytes, dstConfig,
                                           &drawPreference, &tempDrawInfo)) {
        return false;
    }

    sk_sp<GrSurfaceProxy> proxyToRead = sk_ref_sp(srcProxy);
    bool didTempDraw = false;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        if (SkBackingFit::kExact == tempDrawInfo.fTempSurfaceFit) {
            // We only respect this when the entire src is being read. Otherwise we can trigger too
            // many odd ball texture sizes and trash the cache.
            if (width != src->width() || height != src->height()) {
                tempDrawInfo.fTempSurfaceFit= SkBackingFit::kApprox;
            }
        }
        // TODO: Need to decide the semantics of this function for color spaces. Do we support
        // conversion to a passed-in color space? For now, specifying nullptr means that this
        // path will do no conversion, so it will match the behavior of the non-draw path.
        sk_sp<GrRenderTargetContext> tempRTC = fContext->makeRenderTargetContext(
                                                           tempDrawInfo.fTempSurfaceFit,
                                                           tempDrawInfo.fTempSurfaceDesc.fWidth,
                                                           tempDrawInfo.fTempSurfaceDesc.fHeight,
                                                           tempDrawInfo.fTempSurfaceDesc.fConfig,
                                                           nullptr,
                                                           tempDrawInfo.fTempSurfaceDesc.fSampleCnt,
                                                           tempDrawInfo.fTempSurfaceDesc.fOrigin);
        if (tempRTC) {
            SkMatrix textureMatrix = SkMatrix::MakeTrans(SkIntToScalar(left), SkIntToScalar(top));
            sk_sp<GrTextureProxy> proxy = sk_ref_sp(srcProxy->asTextureProxy());
            sk_sp<GrFragmentProcessor> texFP = GrSimpleTextureEffect::Make(
                    fContext->resourceProvider(), proxy, nullptr, textureMatrix);
            sk_sp<GrFragmentProcessor> fp;
            if (unpremul) {
                fp = fContext->createPMToUPMEffect(texFP, proxy->config());
                if (fp) {
                    // We no longer need to do this on CPU after the read back.
                    unpremul = false;
                } else if (GrGpu::kCallerPrefersDraw_DrawPreference == drawPreference) {
                    // We only wanted to do the draw to perform the unpremul so don't bother.
                    tempRTC.reset(nullptr);
                }
            }
            if (tempRTC) {
                if (!fp) {
                    fp = std::move(texFP);
                }
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), tempDrawInfo.fSwizzle);
                SkASSERT(fp);

                GrPaint paint;
                paint.addColorFragmentProcessor(std::move(fp));
                paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                paint.setAllowSRGBInputs(true);
                SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
                tempRTC->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect,
                                  nullptr);
                proxyToRead = tempRTC->asTextureProxyRef();
                left = 0;
                top = 0;
                didTempDraw = true;
            }
        }
    }

    if (!proxyToRead) {
        return false;
    }

    GrSurface* surfaceToRead = proxyToRead->instantiate(fContext->resourceProvider());
    if (!surfaceToRead) {
        return false;
    }

    if (GrGpu::kRequireDraw_DrawPreference == drawPreference && !didTempDraw) {
        return false;
    }
    GrPixelConfig configToRead = dstConfig;
    if (didTempDraw) {
        this->flushSurfaceWrites(proxyToRead.get());
        configToRead = tempDrawInfo.fReadConfig;
    }
    if (!fContext->fGpu->readPixels(surfaceToRead, left, top, width, height, configToRead,
                                    buffer, rowBytes)) {
        return false;
    }

    // Perform umpremul conversion if we weren't able to perform it as a draw.
    if (unpremul) {
        SkColorType colorType;
        if (!GrPixelConfigToColorType(dstConfig, &colorType) ||
            4 != SkColorTypeBytesPerPixel(colorType))
        {
            return false;
        }

        for (int y = 0; y < height; y++) {
            SkUnpremultiplyRow<false>((uint32_t*) buffer, (const uint32_t*) buffer, width);
            buffer = SkTAddOffset<void>(buffer, rowBytes);
        }
    }
    return true;
}

void GrContextPriv::prepareSurfaceForExternalIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    fContext->fDrawingManager->prepareSurfaceForExternalIO(proxy);
}

void GrContextPriv::flushSurfaceWrites(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingWrite()) {
        this->flush(proxy);
    }
}

void GrContextPriv::flushSurfaceIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingIO()) {
        this->flush(proxy);
    }
}

////////////////////////////////////////////////////////////////////////////////
int GrContext::getRecommendedSampleCount(GrPixelConfig config,
                                         SkScalar dpi) const {
    ASSERT_SINGLE_OWNER

    if (!this->caps()->isConfigRenderable(config, true)) {
        return 0;
    }
    int chosenSampleCount = 0;
    if (fGpu->caps()->shaderCaps()->pathRenderingSupport()) {
        if (dpi >= 250.0f) {
            chosenSampleCount = 4;
        } else {
            chosenSampleCount = 16;
        }
    }
    return chosenSampleCount <= fGpu->caps()->maxSampleCount() ? chosenSampleCount : 0;
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeWrappedRenderTargetContext(
                                                               sk_sp<GrRenderTarget> rt,
                                                               sk_sp<SkColorSpace> colorSpace,
                                                               const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(rt)));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    if (proxy->asRenderTargetProxy()) {
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                               std::move(colorSpace), nullptr);
    } else {
        SkASSERT(proxy->asTextureProxy());
        return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
    }
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurface> surface) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));
    if (!proxy) {
        return nullptr;
    }

    return this->makeWrappedSurfaceContext(std::move(proxy), nullptr);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeDeferredSurfaceContext(const GrSurfaceDesc& dstDesc,
                                                                  SkBackingFit fit,
                                                                  SkBudgeted isDstBudgeted) {

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(fContext->resourceProvider(),
                                                               dstDesc, fit, isDstBudgeted);
    if (!proxy) {
        return nullptr;
    }

    return this->makeWrappedSurfaceContext(std::move(proxy), nullptr);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeBackendSurfaceContext(const GrBackendTextureDesc& desc,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurface> surface(fContext->resourceProvider()->wrapBackendTexture(desc));
    if (!surface) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));
    if (!proxy) {
        return nullptr;
    }

    return this->makeWrappedSurfaceContext(std::move(proxy), std::move(colorSpace));
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
                                                                   const GrBackendTextureDesc& desc,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(desc.fFlags & kRenderTarget_GrBackendTextureFlag);

    sk_sp<GrSurface> surface(fContext->resourceProvider()->wrapBackendTexture(desc));
    if (!surface) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
                                                const GrBackendRenderTargetDesc& desc,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrRenderTarget> rt(fContext->resourceProvider()->wrapBackendRenderTarget(desc));
    if (!rt) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(rt)));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureAsRenderTargetRenderTargetContext(
                                                     const GrBackendTextureDesc& desc,
                                                     sk_sp<SkColorSpace> colorSpace,
                                                     const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(desc.fFlags & kRenderTarget_GrBackendTextureFlag);

    sk_sp<GrSurface> surface(fContext->resourceProvider()->wrapBackendTextureAsRenderTarget(desc));
    if (!surface) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

void GrContextPriv::addPreFlushCallbackObject(sk_sp<GrPreFlushCallbackObject> preFlushCBObject) {
    fContext->fDrawingManager->addPreFlushCallbackObject(std::move(preFlushCBObject));
}


static inline GrPixelConfig GrPixelConfigFallback(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
            return kRGBA_8888_GrPixelConfig;
        case kSBGRA_8888_GrPixelConfig:
            return kSRGBA_8888_GrPixelConfig;
        case kAlpha_half_GrPixelConfig:
            return kRGBA_half_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

sk_sp<GrRenderTargetContext> GrContext::makeRenderTargetContextWithFallback(
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted) {
    if (!this->caps()->isConfigRenderable(config, sampleCnt > 0)) {
        config = GrPixelConfigFallback(config);
    }

    return this->makeRenderTargetContext(fit, width, height, config, std::move(colorSpace),
                                         sampleCnt, origin, surfaceProps, budgeted);
}

sk_sp<GrRenderTargetContext> GrContext::makeDeferredRenderTargetContextWithFallback(
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted) {
    if (!this->caps()->isConfigRenderable(config, sampleCnt > 0)) {
        config = GrPixelConfigFallback(config);
    }

    return this->makeDeferredRenderTargetContext(fit, width, height, config, std::move(colorSpace),
                                                 sampleCnt, origin, surfaceProps, budgeted);
}

sk_sp<GrRenderTargetContext> GrContext::makeRenderTargetContext(SkBackingFit fit,
                                                                int width, int height,
                                                                GrPixelConfig config,
                                                                sk_sp<SkColorSpace> colorSpace,
                                                                int sampleCnt,
                                                                GrSurfaceOrigin origin,
                                                                const SkSurfaceProps* surfaceProps,
                                                                SkBudgeted budgeted) {
    if (!this->caps()->isConfigRenderable(config, sampleCnt > 0)) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = origin;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrTexture> tex;
    if (SkBackingFit::kExact == fit) {
        tex = this->resourceProvider()->createTexture(desc, budgeted);
    } else {
        tex.reset(this->resourceProvider()->createApproxTexture(desc, 0));
    }
    if (!tex) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        this->contextPriv().makeWrappedRenderTargetContext(sk_ref_sp(tex->asRenderTarget()),
                                                           std::move(colorSpace), surfaceProps));
    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

sk_sp<GrRenderTargetContext> GrContext::makeDeferredRenderTargetContext(
                                                        SkBackingFit fit,
                                                        int width, int height,
                                                        GrPixelConfig config,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        int sampleCnt,
                                                        GrSurfaceOrigin origin,
                                                        const SkSurfaceProps* surfaceProps,
                                                        SkBudgeted budgeted) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fOrigin = origin;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    sk_sp<GrTextureProxy> rtp = GrSurfaceProxy::MakeDeferred(this->resourceProvider(),
                                                             desc, fit, budgeted);
    if (!rtp) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingManager->makeRenderTargetContext(std::move(rtp),
                                                 std::move(colorSpace),
                                                 surfaceProps));

    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

bool GrContext::abandoned() const {
    ASSERT_SINGLE_OWNER
    return fDrawingManager->wasAbandoned();
}

namespace {
void test_pm_conversions(GrContext* ctx, int* pmToUPMValue, int* upmToPMValue) {
    GrConfigConversionEffect::PMConversion pmToUPM;
    GrConfigConversionEffect::PMConversion upmToPM;
    GrConfigConversionEffect::TestForPreservingPMConversions(ctx, &pmToUPM, &upmToPM);
    *pmToUPMValue = pmToUPM;
    *upmToPMValue = upmToPM;
}
}

void GrContext::testPMConversionsIfNecessary(uint32_t flags) {
    ASSERT_SINGLE_OWNER
    if (SkToBool(GrContextPriv::kUnpremul_PixelOpsFlag & flags)) {
        if (!fDidTestPMConversions) {
            test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
            fDidTestPMConversions = true;
        }
    }
}

sk_sp<GrFragmentProcessor> GrContext::createPMToUPMEffect(sk_sp<GrFragmentProcessor> fp,
                                                          GrPixelConfig config) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    if (kRGBA_half_GrPixelConfig == config) {
        return GrFragmentProcessor::UnpremulOutput(std::move(fp));
    } else if (kRGBA_8888_GrPixelConfig == config || kBGRA_8888_GrPixelConfig == config) {
        GrConfigConversionEffect::PMConversion pmToUPM =
            static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
        if (GrConfigConversionEffect::kPMConversionCnt != pmToUPM) {
            return GrConfigConversionEffect::Make(std::move(fp), pmToUPM);
        }
    }
    return nullptr;
}

sk_sp<GrFragmentProcessor> GrContext::createUPMToPMEffect(sk_sp<GrFragmentProcessor> fp,
                                                          GrPixelConfig config) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    if (kRGBA_half_GrPixelConfig == config) {
        return GrFragmentProcessor::PremulOutput(std::move(fp));
    } else if (kRGBA_8888_GrPixelConfig == config || kBGRA_8888_GrPixelConfig == config) {
        GrConfigConversionEffect::PMConversion upmToPM =
            static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
        if (GrConfigConversionEffect::kPMConversionCnt != upmToPM) {
            return GrConfigConversionEffect::Make(std::move(fp), upmToPM);
        }
    }
    return nullptr;
}

bool GrContext::validPMUPMConversionExists(GrPixelConfig config) const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    // The PM<->UPM tests fail or succeed together so we only need to check one.
    // For F16, we always allow PM/UPM conversion on the GPU, even if it doesn't round-trip.
    return GrConfigConversionEffect::kPMConversionCnt != fPMToUPMConversion ||
           kRGBA_half_GrPixelConfig == config;
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::getResourceCacheLimits(int* maxTextures, size_t* maxTextureBytes) const {
    ASSERT_SINGLE_OWNER
    if (maxTextures) {
        *maxTextures = fResourceCache->getMaxResourceCount();
    }
    if (maxTextureBytes) {
        *maxTextureBytes = fResourceCache->getMaxResourceBytes();
    }
}

void GrContext::setResourceCacheLimits(int maxTextures, size_t maxTextureBytes) {
    ASSERT_SINGLE_OWNER
    fResourceCache->setLimits(maxTextures, maxTextureBytes);
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
}
