/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrContextOptions.h"
#include "GrDrawingManager.h"
#include "GrDrawContext.h"
#include "GrLayerCache.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfacePriv.h"

#include "SkConfig8888.h"
#include "SkGrPriv.h"

#include "batches/GrCopySurfaceBatch.h"
#include "effects/GrConfigConversionEffect.h"
#include "text/GrTextBlobCache.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(&fSingleOwner);)
#define RETURN_IF_ABANDONED if (fDrawingManager->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (fDrawingManager->abandoned()) { return nullptr; }

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
    fFlushToReduceCacheSize = false;
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
    fResourceCache->setOverBudgetCallback(OverBudgetCB, this);
    fResourceProvider = new GrResourceProvider(fGpu, fResourceCache, &fSingleOwner);

    fLayerCache.reset(new GrLayerCache(this));

    fDidTestPMConversions = false;

    GrDrawTarget::Options dtOptions;
    dtOptions.fClipBatchToBounds = options.fClipBatchToBounds;
    dtOptions.fDrawBatchBounds = options.fDrawBatchBounds;
    dtOptions.fMaxBatchLookback = options.fMaxBatchLookback;
    dtOptions.fMaxBatchLookahead = options.fMaxBatchLookahead;
    fDrawingManager.reset(new GrDrawingManager(this, dtOptions, &fSingleOwner));

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

GrContextThreadSafeProxy* GrContext::threadSafeProxy() {
    if (!fThreadSafeProxy) {
        fThreadSafeProxy.reset(new GrContextThreadSafeProxy(fCaps, this->uniqueID()));
    }
    return SkRef(fThreadSafeProxy.get());
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
    fLayerCache->freeAll();
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
    fLayerCache->freeAll();
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
    fLayerCache->freeAll();

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

void GrContext::OverBudgetCB(void* data) {
    SkASSERT(data);

    GrContext* context = reinterpret_cast<GrContext*>(data);

    // Flush the GrBufferedDrawTarget to possibly free up some textures
    context->fFlushToReduceCacheSize = true;
}

void GrContext::TextBlobCacheOverBudgetCB(void* data) {
    SkASSERT(data);

    // Unlike the GrResourceCache, TextBlobs are drawn at the SkGpuDevice level, therefore they
    // cannot use fFlushTorReduceCacheSize because it uses AutoCheckFlush.  The solution is to move
    // drawText calls to below the GrContext level, but this is not trivial because they call
    // drawPath on SkGpuDevice
    GrContext* context = reinterpret_cast<GrContext*>(data);
    context->flush();
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush(int flagsBitfield) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED

    if (kDiscard_FlushBit & flagsBitfield) {
        fDrawingManager->reset();
    } else {
        fDrawingManager->flush();
    }
    fResourceCache->notifyFlushOccurred();
    fFlushToReduceCacheSize = false;
}

bool sw_convert_to_premul(GrPixelConfig srcConfig, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkSrcPixelInfo srcPI;
    if (!GrPixelConfig2ColorAndProfileType(srcConfig, &srcPI.fColorType, nullptr)) {
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
        if (!GrPixelConfigIs8888(srcConfig)) {
            return false;
        }
        applyPremulToSrc = true;
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

    SkAutoTUnref<GrTexture> tempTexture;
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
        SkAutoTUnref<const GrFragmentProcessor> fp;
        SkMatrix textureMatrix;
        textureMatrix.setIDiv(tempTexture->width(), tempTexture->height());
        if (applyPremulToSrc) {
            fp.reset(this->createUPMToPMEffect(tempTexture, tempDrawInfo.fSwizzle,
                                               textureMatrix));
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
                fp.reset(GrConfigConversionEffect::Create(tempTexture, tempDrawInfo.fSwizzle,
                    GrConfigConversionEffect::kNone_PMConversion, textureMatrix));
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
            if (!fGpu->writePixels(tempTexture, 0, 0, width, height,
                                   tempDrawInfo.fWriteConfig, buffer,
                                   rowBytes)) {
                return false;
            }
            SkMatrix matrix;
            matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(renderTarget));
            if (!drawContext) {
                return false;
            }
            // SRGBTODO: AllowSRGBInputs? (We could force it on here, so we don't need the
            // per-texture override in config conversion effect?)
            GrPaint paint;
            paint.addColorFragmentProcessor(fp);
            paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
            SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
            drawContext->drawRect(GrClip::WideOpen(), paint, matrix, rect, nullptr);

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
    if (unpremul && !GrPixelConfigIs8888(dstConfig)) {
        // The unpremul flag is only allowed for 8888 configs.
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

    SkAutoTUnref<GrSurface> surfaceToRead(SkRef(src));
    bool didTempDraw = false;
    if (GrGpu::kNoDraw_DrawPreference != drawPreference) {
        if (tempDrawInfo.fUseExactScratch) {
            // We only respect this when the entire src is being read. Otherwise we can trigger too
            // many odd ball texture sizes and trash the cache.
            if (width != src->width() || height != src->height()) {
                tempDrawInfo.fUseExactScratch = false;
            }
        }
        SkAutoTUnref<GrTexture> temp;
        if (tempDrawInfo.fUseExactScratch) {
            temp.reset(this->textureProvider()->createTexture(tempDrawInfo.fTempSurfaceDesc,
                                                              SkBudgeted::kYes));
        } else {
            temp.reset(this->textureProvider()->createApproxTexture(tempDrawInfo.fTempSurfaceDesc));
        }
        if (temp) {
            SkMatrix textureMatrix;
            textureMatrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            textureMatrix.postIDiv(src->width(), src->height());
            SkAutoTUnref<const GrFragmentProcessor> fp;
            if (unpremul) {
                fp.reset(this->createPMToUPMEffect(src->asTexture(), tempDrawInfo.fSwizzle,
                    textureMatrix));
                if (fp) {
                    unpremul = false; // we no longer need to do this on CPU after the read back.
                } else if (GrGpu::kCallerPrefersDraw_DrawPreference == drawPreference) {
                    // We only wanted to do the draw in order to perform the unpremul so don't
                    // bother.
                    temp.reset(nullptr);
                }
            }
            if (!fp && temp) {
                fp.reset(GrConfigConversionEffect::Create(src->asTexture(), tempDrawInfo.fSwizzle,
                    GrConfigConversionEffect::kNone_PMConversion, textureMatrix));
            }
            if (fp) {
                // SRGBTODO: AllowSRGBInputs? (We could force it on here, so we don't need the
                // per-texture override in config conversion effect?)
                GrPaint paint;
                paint.addColorFragmentProcessor(fp);
                paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
                SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
                SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(temp->asRenderTarget()));
                drawContext->drawRect(GrClip::WideOpen(), paint, SkMatrix::I(), rect, nullptr);
                surfaceToRead.reset(SkRef(temp.get()));
                left = 0;
                top = 0;
                didTempDraw = true;
            }
        }
    }

    if (GrGpu::kRequireDraw_DrawPreference == drawPreference && !didTempDraw) {
        return false;
    }
    GrPixelConfig configToRead = dstConfig;
    if (didTempDraw) {
        this->flushSurfaceWrites(surfaceToRead);
        configToRead = tempDrawInfo.fReadConfig;
    }
    if (!fGpu->readPixels(surfaceToRead, left, top, width, height, configToRead, buffer,
                           rowBytes)) {
        return false;
    }

    // Perform umpremul conversion if we weren't able to perform it as a draw.
    if (unpremul) {
        SkDstPixelInfo dstPI;
        if (!GrPixelConfig2ColorAndProfileType(dstConfig, &dstPI.fColorType, nullptr)) {
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
    if (surface->surfacePriv().hasPendingIO()) {
        this->flush();
    }
    GrRenderTarget* rt = surface->asRenderTarget();
    if (fGpu && rt) {
        fGpu->resolveRenderTarget(rt);
    }
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

    if (!dst->asRenderTarget()) {
        SkIRect clippedSrcRect;
        SkIPoint clippedDstPoint;
        if (!GrCopySurfaceBatch::ClipSrcRectAndDstPoint(dst, src, srcRect, dstPoint,
                                                        &clippedSrcRect, &clippedDstPoint)) {
            return false;
        }
        // If we don't have an RT for the dst then we won't have a GrDrawContext to insert the
        // the copy surface into. In the future we plan to have a more limited Context type
        // (GrCopyContext?) that has the subset of GrDrawContext operations that should be
        // allowed on textures that aren't render targets.
        // For now we just flush any writes to the src and issue an immediate copy to the dst.
        src->flushWrites();
        return fGpu->copySurface(dst, src, clippedSrcRect, clippedDstPoint);
    }
    SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(dst->asRenderTarget()));
    if (!drawContext) {
        return false;
    }

    if (!drawContext->copySurface(src, srcRect, dstPoint)) {
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


GrDrawContext* GrContext::drawContext(GrRenderTarget* rt, const SkSurfaceProps* surfaceProps) {
    ASSERT_SINGLE_OWNER
    return fDrawingManager->drawContext(rt, surfaceProps);
}

bool GrContext::abandoned() const {
    ASSERT_SINGLE_OWNER
    return fDrawingManager->abandoned();
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

const GrFragmentProcessor* GrContext::createPMToUPMEffect(GrTexture* texture,
                                                          const GrSwizzle& swizzle,
                                                          const SkMatrix& matrix) const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion pmToUPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != pmToUPM) {
        return GrConfigConversionEffect::Create(texture, swizzle, pmToUPM, matrix);
    } else {
        return nullptr;
    }
}

const GrFragmentProcessor* GrContext::createUPMToPMEffect(GrTexture* texture,
                                                          const GrSwizzle& swizzle,
                                                          const SkMatrix& matrix) const {
    ASSERT_SINGLE_OWNER
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion upmToPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != upmToPM) {
        return GrConfigConversionEffect::Create(texture, swizzle, upmToPM, matrix);
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
