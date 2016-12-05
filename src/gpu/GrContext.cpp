/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrContextOptions.h"
#include "GrDrawingManager.h"
#include "GrRenderTargetContext.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetProxy.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfaceContext.h"
#include "GrSurfacePriv.h"
#include "GrTextureContext.h"

#include "SkConfig8888.h"
#include "SkGrPriv.h"

#include "batches/GrCopySurfaceBatch.h"
#include "effects/GrConfigConversionEffect.h"
#include "effects/GrGammaEffect.h"
#include "text/GrTextBlobCache.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fSingleOwner);)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fContext->fSingleOwner);)
#define RETURN_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return false; }
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
    fBatchFontCache = nullptr;
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

    fDidTestPMConversions = false;

    GrRenderTargetOpList::Options rtOpListOptions;
    rtOpListOptions.fClipBatchToBounds = options.fClipBatchToBounds;
    rtOpListOptions.fMaxBatchLookback = options.fMaxBatchLookback;
    rtOpListOptions.fMaxBatchLookahead = options.fMaxBatchLookahead;
    GrPathRendererChain::Options prcOptions;
    prcOptions.fDisableDistanceFieldRenderer = options.fDisableDistanceFieldPaths;
    prcOptions.fAllowPathMaskCaching = options.fAllowPathMaskCaching;
    prcOptions.fDisableAllPathRenderers = options.fForceSWPathMasks;
    fDrawingManager.reset(new GrDrawingManager(this, rtOpListOptions, prcOptions,
                                               options.fImmediateMode, &fSingleOwner));

    // GrBatchFontCache will eventually replace GrFontCache
    fBatchFontCache = new GrBatchFontCache(this);

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
    delete fBatchFontCache;

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

    fBatchFontCache->freeAll();
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

    fBatchFontCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    this->flush();

    fBatchFontCache->freeAll();

    fDrawingManager->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
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
    fDrawingManager->flush();
}

bool sw_convert_to_premul(GrPixelConfig srcConfig, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkSrcPixelInfo srcPI;
    if (!GrPixelConfigToColorType(srcConfig, &srcPI.fColorType)) {
        return false;
    }
    srcPI.fAlphaType = kUnpremul_SkAlphaType;
    srcPI.fPixels = inPixels;
    srcPI.fRowBytes = inRowBytes;

    SkDstPixelInfo dstPI;
    dstPI.fColorType = srcPI.fColorType;
    dstPI.fAlphaType = kPremul_SkAlphaType;
    dstPI.fPixels = outPixels;
    dstPI.fRowBytes = outRowBytes;

    return srcPI.convertPixelsTo(&dstPI, width, height);
}

bool GrContext::writeSurfacePixels(GrSurface* surface,
                                   int left, int top, int width, int height,
                                   GrPixelConfig srcConfig, const void* buffer, size_t rowBytes,
                                   uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    ASSERT_OWNED_RESOURCE(surface);
    SkASSERT(surface);
    GR_AUDIT_TRAIL_AUTO_FRAME(&fAuditTrail, "GrContext::writeSurfacePixels");

    this->testPMConversionsIfNecessary(pixelOpsFlags);

    // Trim the params here so that if we wind up making a temporary surface it can be as small as
    // necessary and because GrGpu::getWritePixelsInfo requires it.
    if (!GrSurfacePriv::AdjustWritePixelParams(surface->width(), surface->height(),
                                               GrBytesPerPixel(srcConfig), &left, &top, &width,
                                               &height, &buffer, &rowBytes)) {
        return false;
    }

    bool applyPremulToSrc = false;
    if (kUnpremul_PixelOpsFlag & pixelOpsFlags) {
        if (!GrPixelConfigIs8888Unorm(srcConfig)) {
            return false;
        }
        applyPremulToSrc = true;
    }
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(surface->config()) != GrPixelConfigIsSint(srcConfig)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
    // Don't prefer to draw for the conversion (and thereby access a texture from the cache) when
    // we've already determined that there isn't a roundtrip preserving conversion processor pair.
    if (applyPremulToSrc && !this->didFailPMUPMConversionTest()) {
        drawPreference = GrGpu::kCallerPrefersDraw_DrawPreference;
    }

    GrGpu::WritePixelTempDrawInfo tempDrawInfo;
    if (!fGpu->getWritePixelsInfo(surface, width, height, srcConfig, &drawPreference,
                                  &tempDrawInfo)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) && surface->surfacePriv().hasPendingIO()) {
        this->flush();
    }

    sk_sp<GrTexture> tempTexture;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        tempTexture.reset(
            this->textureProvider()->createApproxTexture(tempDrawInfo.fTempSurfaceDesc));
        if (!tempTexture && GrGpu::kRequireDraw_DrawPreference == drawPreference) {
            return false;
        }
    }

    // temp buffer for doing sw premul conversion, if needed.
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);
    if (tempTexture) {
        sk_sp<GrFragmentProcessor> fp;
        SkMatrix textureMatrix;
        textureMatrix.setIDiv(tempTexture->width(), tempTexture->height());
        if (applyPremulToSrc) {
            fp = this->createUPMToPMEffect(tempTexture.get(), tempDrawInfo.fSwizzle, textureMatrix);
            // If premultiplying was the only reason for the draw, fall back to a straight write.
            if (!fp) {
                if (GrGpu::kCallerPrefersDraw_DrawPreference == drawPreference) {
                    tempTexture.reset(nullptr);
                }
            } else {
                applyPremulToSrc = false;
            }
        }
        if (tempTexture) {
            if (!fp) {
                fp = GrConfigConversionEffect::Make(tempTexture.get(), tempDrawInfo.fSwizzle,
                                                    GrConfigConversionEffect::kNone_PMConversion,
                                                    textureMatrix);
                if (!fp) {
                    return false;
                }
            }
            GrRenderTarget* renderTarget = surface->asRenderTarget();
            SkASSERT(renderTarget);
            if (tempTexture->surfacePriv().hasPendingIO()) {
                this->flush();
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
            if (!fGpu->writePixels(tempTexture.get(), 0, 0, width, height,
                                   tempDrawInfo.fWriteConfig, buffer,
                                   rowBytes)) {
                return false;
            }
            SkMatrix matrix;
            matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            // TODO: Need to decide the semantics of this function for color spaces. Do we support
            // conversion from a passed-in color space? For now, specifying nullptr means that this
            // path will do no conversion, so it will match the behavior of the non-draw path.
            sk_sp<GrRenderTargetContext> renderTargetContext(
                this->contextPriv().makeWrappedRenderTargetContext(sk_ref_sp(renderTarget),
                                                                   nullptr));
            if (!renderTargetContext) {
                return false;
            }
            GrPaint paint;
            paint.addColorFragmentProcessor(std::move(fp));
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            paint.setAllowSRGBInputs(true);
            SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
            renderTargetContext->drawRect(GrNoClip(), paint, GrAA::kNo, matrix, rect, nullptr);

            if (kFlushWrites_PixelOp & pixelOpsFlags) {
                this->flushSurfaceWrites(surface);
            }
        }
    }
    if (!tempTexture) {
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
        return fGpu->writePixels(surface, left, top, width, height, srcConfig, buffer, rowBytes);
    }
    return true;
}

bool GrContext::readSurfacePixels(GrSurface* src,
                                  int left, int top, int width, int height,
                                  GrPixelConfig dstConfig, void* buffer, size_t rowBytes,
                                  uint32_t flags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    ASSERT_OWNED_RESOURCE(src);
    SkASSERT(src);
    GR_AUDIT_TRAIL_AUTO_FRAME(&fAuditTrail, "GrContext::readSurfacePixels");

    this->testPMConversionsIfNecessary(flags);
    SkAutoMutexAcquire ama(fReadPixelsMutex);

    // Adjust the params so that if we wind up using an intermediate surface we've already done
    // all the trimming and the temporary can be the min size required.
    if (!GrSurfacePriv::AdjustReadPixelParams(src->width(), src->height(),
                                              GrBytesPerPixel(dstConfig), &left,
                                              &top, &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    if (!(kDontFlush_PixelOpsFlag & flags) && src->surfacePriv().hasPendingWrite()) {
        this->flush();
    }

    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & flags);
    if (unpremul && !GrPixelConfigIs8888Unorm(dstConfig)) {
        // The unpremul flag is only allowed for 8888 configs.
        return false;
    }
    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(src->config()) != GrPixelConfigIsSint(dstConfig)) {
        return false;
    }

    GrGpu::DrawPreference drawPreference = GrGpu::kNoDraw_DrawPreference;
    // Don't prefer to draw for the conversion (and thereby access a texture from the cache) when
    // we've already determined that there isn't a roundtrip preserving conversion processor pair.
    if (unpremul && !this->didFailPMUPMConversionTest()) {
        drawPreference = GrGpu::kCallerPrefersDraw_DrawPreference;
    }

    GrGpu::ReadPixelTempDrawInfo tempDrawInfo;
    if (!fGpu->getReadPixelsInfo(src, width, height, rowBytes, dstConfig, &drawPreference,
                                 &tempDrawInfo)) {
        return false;
    }

    sk_sp<GrSurface> surfaceToRead(SkRef(src));
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
        sk_sp<GrRenderTargetContext> tempRTC = this->makeRenderTargetContext(
                                                           tempDrawInfo.fTempSurfaceFit,
                                                           tempDrawInfo.fTempSurfaceDesc.fWidth,
                                                           tempDrawInfo.fTempSurfaceDesc.fHeight,
                                                           tempDrawInfo.fTempSurfaceDesc.fConfig,
                                                           nullptr,
                                                           tempDrawInfo.fTempSurfaceDesc.fSampleCnt,
                                                           tempDrawInfo.fTempSurfaceDesc.fOrigin);
        if (tempRTC) {
            SkMatrix textureMatrix;
            textureMatrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            textureMatrix.postIDiv(src->width(), src->height());
            sk_sp<GrFragmentProcessor> fp;
            if (unpremul) {
                fp = this->createPMToUPMEffect(src->asTexture(), tempDrawInfo.fSwizzle,
                                               textureMatrix);
                if (fp) {
                    unpremul = false; // we no longer need to do this on CPU after the read back.
                } else if (GrGpu::kCallerPrefersDraw_DrawPreference == drawPreference) {
                    // We only wanted to do the draw in order to perform the unpremul so don't
                    // bother.
                    tempRTC.reset(nullptr);
                }
            }
            if (!fp && tempRTC) {
                fp = GrConfigConversionEffect::Make(src->asTexture(), tempDrawInfo.fSwizzle,
                                                    GrConfigConversionEffect::kNone_PMConversion,
                                                    textureMatrix);
            }
            if (fp) {
                GrPaint paint;
                paint.addColorFragmentProcessor(std::move(fp));
                paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                paint.setAllowSRGBInputs(true);
                SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
                tempRTC->drawRect(GrNoClip(), paint, GrAA::kNo, SkMatrix::I(), rect, nullptr);
                surfaceToRead.reset(tempRTC->asTexture().release());
                left = 0;
                top = 0;
                didTempDraw = true;
            }
        }
    }

    if (!surfaceToRead) {
        return false;
    }

    if (GrGpu::kRequireDraw_DrawPreference == drawPreference && !didTempDraw) {
        return false;
    }
    GrPixelConfig configToRead = dstConfig;
    if (didTempDraw) {
        this->flushSurfaceWrites(surfaceToRead.get());
        configToRead = tempDrawInfo.fReadConfig;
    }
    if (!fGpu->readPixels(surfaceToRead.get(), left, top, width, height, configToRead, buffer,
                          rowBytes)) {
        return false;
    }

    // Perform umpremul conversion if we weren't able to perform it as a draw.
    if (unpremul) {
        SkDstPixelInfo dstPI;
        if (!GrPixelConfigToColorType(dstConfig, &dstPI.fColorType)) {
            return false;
        }
        dstPI.fAlphaType = kUnpremul_SkAlphaType;
        dstPI.fPixels = buffer;
        dstPI.fRowBytes = rowBytes;

        SkSrcPixelInfo srcPI;
        srcPI.fColorType = dstPI.fColorType;
        srcPI.fAlphaType = kPremul_SkAlphaType;
        srcPI.fPixels = buffer;
        srcPI.fRowBytes = rowBytes;

        return srcPI.convertPixelsTo(&dstPI, width, height);
    }
    return true;
}

void GrContext::prepareSurfaceForExternalIO(GrSurface* surface) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkASSERT(surface);
    ASSERT_OWNED_RESOURCE(surface);
    fDrawingManager->prepareSurfaceForExternalIO(surface);
}

bool GrContext::copySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    GR_AUDIT_TRAIL_AUTO_FRAME(&fAuditTrail, "GrContext::copySurface");

    if (!src || !dst) {
        return false;
    }
    ASSERT_OWNED_RESOURCE(src);
    ASSERT_OWNED_RESOURCE(dst);

    // We don't allow conversion between integer configs and float/fixed configs.
    if (GrPixelConfigIsSint(dst->config()) != GrPixelConfigIsSint(src->config())) {
        return false;
    }

#ifndef ENABLE_MDB
    // We can't yet fully defer copies to textures, so GrTextureContext::copySurface will
    // execute the copy immediately. Ensure the data is ready.
    src->flushWrites();
#endif

    sk_sp<GrSurfaceContext> surfaceContext(
        this->contextPriv().makeWrappedSurfaceContext(sk_ref_sp(dst)));

    if (!surfaceContext) {
        return false;
    }

    if (!surfaceContext->copySurface(src, srcRect, dstPoint)) {
        return false;
    }

    return true;
}

void GrContext::flushSurfaceWrites(GrSurface* surface) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    if (surface->surfacePriv().hasPendingWrite()) {
        this->flush();
    }
}

void GrContext::flushSurfaceIO(GrSurface* surface) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    if (surface->surfacePriv().hasPendingIO()) {
        this->flush();
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

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(sk_sp<GrSurface> surface) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));

    if (proxy->asRenderTargetProxy()) {
        return this->drawingManager()->makeRenderTargetContext(std::move(proxy), nullptr, nullptr);
    } else {
        SkASSERT(proxy->asTextureProxy());
        return this->drawingManager()->makeTextureContext(std::move(proxy));
    }
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
                                                                   const GrBackendTextureDesc& desc, 
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props,
                                                                   GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(desc.fFlags & kRenderTarget_GrBackendTextureFlag);

    sk_sp<GrSurface> surface(fContext->textureProvider()->wrapBackendTexture(desc, ownership));
    if (!surface) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(surface)));

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
                                                const GrBackendRenderTargetDesc& desc,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrRenderTarget> rt(fContext->textureProvider()->wrapBackendRenderTarget(desc));
    if (!rt) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(rt)));

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

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

static inline GrPixelConfig GrPixelConfigFallback(GrPixelConfig config) {
    static const GrPixelConfig kFallback[] = {
        kUnknown_GrPixelConfig,        // kUnknown_GrPixelConfig
        kRGBA_8888_GrPixelConfig,      // kAlpha_8_GrPixelConfig
        kUnknown_GrPixelConfig,        // kIndex_8_GrPixelConfig
        kRGBA_8888_GrPixelConfig,      // kRGB_565_GrPixelConfig
        kRGBA_8888_GrPixelConfig,      // kRGBA_4444_GrPixelConfig
        kUnknown_GrPixelConfig,        // kRGBA_8888_GrPixelConfig
        kRGBA_8888_GrPixelConfig,      // kBGRA_8888_GrPixelConfig
        kUnknown_GrPixelConfig,        // kSRGBA_8888_GrPixelConfig
        kSRGBA_8888_GrPixelConfig,     // kSBGRA_8888_GrPixelConfig
        kUnknown_GrPixelConfig,        // kRGBA_8888_sint_GrPixelConfig
        kUnknown_GrPixelConfig,        // kETC1_GrPixelConfig
        kUnknown_GrPixelConfig,        // kLATC_GrPixelConfig
        kUnknown_GrPixelConfig,        // kR11_EAC_GrPixelConfig
        kUnknown_GrPixelConfig,        // kASTC_12x12_GrPixelConfig
        kUnknown_GrPixelConfig,        // kRGBA_float_GrPixelConfig
        kRGBA_half_GrPixelConfig,      // kAlpha_half_GrPixelConfig
        kUnknown_GrPixelConfig,        // kRGBA_half_GrPixelConfig
    };
    return kFallback[config];

    GR_STATIC_ASSERT(0  == kUnknown_GrPixelConfig);
    GR_STATIC_ASSERT(1  == kAlpha_8_GrPixelConfig);
    GR_STATIC_ASSERT(2  == kIndex_8_GrPixelConfig);
    GR_STATIC_ASSERT(3  == kRGB_565_GrPixelConfig);
    GR_STATIC_ASSERT(4  == kRGBA_4444_GrPixelConfig);
    GR_STATIC_ASSERT(5  == kRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(6  == kBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(7  == kSRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(8  == kSBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(9  == kRGBA_8888_sint_GrPixelConfig);
    GR_STATIC_ASSERT(10 == kETC1_GrPixelConfig);
    GR_STATIC_ASSERT(11 == kLATC_GrPixelConfig);
    GR_STATIC_ASSERT(12 == kR11_EAC_GrPixelConfig);
    GR_STATIC_ASSERT(13 == kASTC_12x12_GrPixelConfig);
    GR_STATIC_ASSERT(14 == kRGBA_float_GrPixelConfig);
    GR_STATIC_ASSERT(15 == kAlpha_half_GrPixelConfig);
    GR_STATIC_ASSERT(16 == kRGBA_half_GrPixelConfig);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kFallback) == kGrPixelConfigCnt);
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
        tex.reset(this->textureProvider()->createTexture(desc, budgeted));
    } else {
        tex.reset(this->textureProvider()->createApproxTexture(desc));
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

    sk_sp<GrSurfaceProxy> rtp = GrSurfaceProxy::MakeDeferred(*this->caps(), desc, fit, budgeted);

    return fDrawingManager->makeRenderTargetContext(std::move(rtp),
                                                    std::move(colorSpace),
                                                    surfaceProps);
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
    if (SkToBool(kUnpremul_PixelOpsFlag & flags)) {
        SkAutoMutexAcquire ama(fTestPMConversionsMutex);
        if (!fDidTestPMConversions) {
            test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
            fDidTestPMConversions = true;
        }
    }
}

sk_sp<GrFragmentProcessor> GrContext::createPMToUPMEffect(GrTexture* texture,
                                                          const GrSwizzle& swizzle,
                                                          const SkMatrix& matrix) const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion pmToUPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != pmToUPM) {
        return GrConfigConversionEffect::Make(texture, swizzle, pmToUPM, matrix);
    } else {
        return nullptr;
    }
}

sk_sp<GrFragmentProcessor> GrContext::createUPMToPMEffect(GrTexture* texture,
                                                          const GrSwizzle& swizzle,
                                                          const SkMatrix& matrix) const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion upmToPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != upmToPM) {
        return GrConfigConversionEffect::Make(texture, swizzle, upmToPM, matrix);
    } else {
        return nullptr;
    }
}

bool GrContext::didFailPMUPMConversionTest() const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return GrConfigConversionEffect::kNone_PMConversion == fPMToUPMConversion;
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
