
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"

#include "GrBatchFontCache.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrCaps.h"
#include "GrContextOptions.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawingManager.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrGpuResource.h"
#include "GrGpuResourcePriv.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrLayerCache.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"
#include "GrPathUtils.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrStencilAndCoverTextContext.h"
#include "GrStrokeInfo.h"
#include "GrSurfacePriv.h"
#include "GrTextBlobCache.h"
#include "GrTexturePriv.h"
#include "GrTracing.h"
#include "GrVertices.h"
#include "SkDashPathPriv.h"
#include "SkConfig8888.h"
#include "SkGrPriv.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "SkSurfacePriv.h"
#include "SkTLazy.h"
#include "SkTLS.h"
#include "SkTraceEvent.h"
#include "SkTTopoSort.h"

#include "batches/GrBatch.h"

#include "effects/GrConfigConversionEffect.h"
#include "effects/GrDashingEffect.h"
#include "effects/GrSingleTextureEffect.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define RETURN_IF_ABANDONED if (fDrawingManager->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (fDrawingManager->abandoned()) { return nullptr; }


////////////////////////////////////////////////////////////////////////////////

void GrDrawingManager::cleanup() {
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->unref();
    }

    fDrawTargets.reset();

    delete fNVPRTextContext;
    fNVPRTextContext = nullptr;

    for (int i = 0; i < kNumPixelGeometries; ++i) {
        delete fTextContexts[i][0];
        fTextContexts[i][0] = nullptr;
        delete fTextContexts[i][1];
        fTextContexts[i][1] = nullptr;
    }
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup();
}

void GrDrawingManager::abandon() {
    fAbandoned = true;
    this->cleanup();
}

void GrDrawingManager::reset() {
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->reset();
    }
}

void GrDrawingManager::flush() {
    SkDEBUGCODE(bool result =) 
                        SkTTopoSort<GrDrawTarget, GrDrawTarget::TopoSortTraits>(&fDrawTargets);
    SkASSERT(result);

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->flush();
    }

#ifndef ENABLE_MDB
    // When MDB is disabled we keep reusing the same drawTarget
    if (fDrawTargets.count()) {
        SkASSERT(fDrawTargets.count() == 1);
        fDrawTargets[0]->resetFlag(GrDrawTarget::kWasOutput_Flag);
    }
#endif
}

GrTextContext* GrDrawingManager::textContext(const SkSurfaceProps& props,
                                             GrRenderTarget* rt) {
    if (this->abandoned()) {
        return nullptr;
    }

    SkASSERT(props.pixelGeometry() < kNumPixelGeometries);
    bool useDIF = props.isUseDeviceIndependentFonts();

    if (useDIF && fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        rt->isStencilBufferMultisampled()) {
        GrStencilAttachment* sb = fContext->resourceProvider()->attachStencilAttachment(rt);
        if (sb) {
            if (!fNVPRTextContext) {
                fNVPRTextContext = GrStencilAndCoverTextContext::Create(fContext, props);
            }

            return fNVPRTextContext;
        }
    }

    if (!fTextContexts[props.pixelGeometry()][useDIF]) {
        fTextContexts[props.pixelGeometry()][useDIF] = GrAtlasTextContext::Create(fContext, props);
    }

    return fTextContexts[props.pixelGeometry()][useDIF];
}

GrDrawTarget* GrDrawingManager::newDrawTarget(GrRenderTarget* rt) {
    SkASSERT(fContext);

#ifndef ENABLE_MDB
    // When MDB is disabled we always just return the single drawTarget
    if (fDrawTargets.count()) {
        SkASSERT(fDrawTargets.count() == 1);
        // DrawingManager gets the creation ref - this ref is for the caller
        return SkRef(fDrawTargets[0]);
    }
#endif

    GrDrawTarget* dt = new GrDrawTarget(fContext->getGpu(), fContext->resourceProvider());

    *fDrawTargets.append() = dt;

    // DrawingManager gets the creation ref - this ref is for the caller 
    return SkRef(dt);
}

GrDrawContext* GrDrawingManager::drawContext(GrRenderTarget* rt, 
                                             const SkSurfaceProps* surfaceProps) {
    if (this->abandoned()) {
        return nullptr;
    }

    return new GrDrawContext(this, rt, surfaceProps);
}

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
    fPathRendererChain = nullptr;
    fSoftwarePathRenderer = nullptr;
    fBatchFontCache = nullptr;
    fFlushToReduceCacheSize = false;
}

bool GrContext::init(GrBackend backend, GrBackendContext backendContext,
                     const GrContextOptions& options) {
    SkASSERT(!fGpu);

    fGpu = GrGpu::Create(backend, backendContext, options, this);
    if (!fGpu) {
        return false;
    }
    this->initCommon();
    return true;
}

void GrContext::initCommon() {
    fCaps = SkRef(fGpu->caps());
    fResourceCache = new GrResourceCache(fCaps);
    fResourceCache->setOverBudgetCallback(OverBudgetCB, this);
    fResourceProvider = new GrResourceProvider(fGpu, fResourceCache);

    fLayerCache.reset(new GrLayerCache(this));

    fDidTestPMConversions = false;

    fDrawingManager.reset(new GrDrawingManager(this));

    // GrBatchFontCache will eventually replace GrFontCache
    fBatchFontCache = new GrBatchFontCache(this);

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB, this));
}

GrContext::~GrContext() {
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
    SkSafeUnref(fPathRendererChain);
    SkSafeUnref(fSoftwarePathRenderer);
}

void GrContext::abandonContext() {
    fResourceProvider->abandon();
    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->contextAbandoned();

    // a path renderer may be holding onto resources that
    // are now unusable
    SkSafeSetNull(fPathRendererChain);
    SkSafeSetNull(fSoftwarePathRenderer);

    fDrawingManager->abandon();

    fBatchFontCache->freeAll();
    fLayerCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    this->flush();

    fBatchFontCache->freeAll();
    fLayerCache->freeAll();
    // a path renderer may be holding onto resources
    SkSafeSetNull(fPathRendererChain);
    SkSafeSetNull(fSoftwarePathRenderer);

    fResourceCache->purgeAllUnlocked();
}

void GrContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
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
    RETURN_FALSE_IF_ABANDONED
    ASSERT_OWNED_RESOURCE(surface);
    SkASSERT(surface);

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
    if (!fGpu->getWritePixelsInfo(surface, width, height, rowBytes, srcConfig, &drawPreference,
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
#if defined(GOOGLE3)
    // Stack frame size is limited in GOOGLE3.
    SkAutoSTMalloc<48 * 48, uint32_t> tmpPixels(0);
#else
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);
#endif
    if (tempTexture) {
        SkAutoTUnref<const GrFragmentProcessor> fp;
        SkMatrix textureMatrix;
        textureMatrix.setIDiv(tempTexture->width(), tempTexture->height());
        GrPaint paint;
        if (applyPremulToSrc) {
            fp.reset(this->createUPMToPMEffect(tempTexture, tempDrawInfo.fSwapRAndB,
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
                fp.reset(GrConfigConversionEffect::Create(tempTexture, tempDrawInfo.fSwapRAndB,
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
                                   tempDrawInfo.fTempSurfaceDesc.fConfig, buffer,
                                   rowBytes)) {
                return false;
            }
            SkMatrix matrix;
            matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(renderTarget));
            if (!drawContext) {
                return false;
            }
            paint.addColorFragmentProcessor(fp);
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
    RETURN_FALSE_IF_ABANDONED
    ASSERT_OWNED_RESOURCE(src);
    SkASSERT(src);

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
            temp.reset(this->textureProvider()->createTexture(tempDrawInfo.fTempSurfaceDesc, true));
        } else {
            temp.reset(this->textureProvider()->createApproxTexture(tempDrawInfo.fTempSurfaceDesc));
        }
        if (temp) {
            SkMatrix textureMatrix;
            textureMatrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
            textureMatrix.postIDiv(src->width(), src->height());
            GrPaint paint;
            SkAutoTUnref<const GrFragmentProcessor> fp;
            if (unpremul) {
                fp.reset(this->createPMToUPMEffect(src->asTexture(), tempDrawInfo.fSwapRAndB,
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
                fp.reset(GrConfigConversionEffect::Create(src->asTexture(), tempDrawInfo.fSwapRAndB,
                    GrConfigConversionEffect::kNone_PMConversion, textureMatrix));
            }
            if (fp) {
                paint.addColorFragmentProcessor(fp);
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
        // We swapped R and B while doing the temp draw. Swap back on the read.
        if (tempDrawInfo.fSwapRAndB) {
            configToRead = GrPixelConfigSwapRAndB(dstConfig);
        }
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

void GrContext::copySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint, uint32_t pixelOpsFlags) {
    RETURN_IF_ABANDONED
    if (!src || !dst) {
        return;
    }
    ASSERT_OWNED_RESOURCE(src);
    ASSERT_OWNED_RESOURCE(dst);

    // Since we're going to the draw target and not GPU, no need to check kNoFlush
    // here.
    if (!dst->asRenderTarget()) {
        return;
    }

    SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(dst->asRenderTarget()));
    if (!drawContext) {
        return;
    }

    drawContext->copySurface(src, srcRect, dstPoint);

    if (kFlushWrites_PixelOp & pixelOpsFlags) {
        this->flush();
    }
}

void GrContext::flushSurfaceWrites(GrSurface* surface) {
    RETURN_IF_ABANDONED
    if (surface->surfacePriv().hasPendingWrite()) {
        this->flush();
    }
}

/*
 * This method finds a path renderer that can draw the specified path on
 * the provided target.
 * Due to its expense, the software path renderer has split out so it can
 * can be individually allowed/disallowed via the "allowSW" boolean.
 */
GrPathRenderer* GrContext::getPathRenderer(const GrDrawTarget* target,
                                           const GrPipelineBuilder* pipelineBuilder,
                                           const SkMatrix& viewMatrix,
                                           const SkPath& path,
                                           const GrStrokeInfo& stroke,
                                           bool allowSW,
                                           GrPathRendererChain::DrawType drawType,
                                           GrPathRendererChain::StencilSupport* stencilSupport) {

    if (!fPathRendererChain) {
        fPathRendererChain = new GrPathRendererChain(this);
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(target,
                                                             pipelineBuilder,
                                                             viewMatrix,
                                                             path,
                                                             stroke,
                                                             drawType,
                                                             stencilSupport);

    if (!pr && allowSW) {
        if (!fSoftwarePathRenderer) {
            fSoftwarePathRenderer = new GrSoftwarePathRenderer(this);
        }
        pr = fSoftwarePathRenderer;
    }

    return pr;
}

////////////////////////////////////////////////////////////////////////////////
int GrContext::getRecommendedSampleCount(GrPixelConfig config,
                                         SkScalar dpi) const {
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
    return chosenSampleCount <= fGpu->caps()->maxSampleCount() ?
        chosenSampleCount : 0;
}


GrDrawContext* GrContext::drawContext(GrRenderTarget* rt, const SkSurfaceProps* surfaceProps) {
    return fDrawingManager->drawContext(rt, surfaceProps);
}

bool GrContext::abandoned() const { 
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
    if (SkToBool(kUnpremul_PixelOpsFlag & flags)) {
        SkAutoMutexAcquire ama(fTestPMConversionsMutex);
        if (!fDidTestPMConversions) {
            test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
            fDidTestPMConversions = true;
        }
    }
}

const GrFragmentProcessor* GrContext::createPMToUPMEffect(GrTexture* texture,
                                                          bool swapRAndB,
                                                          const SkMatrix& matrix) const {
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion pmToUPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != pmToUPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, pmToUPM, matrix);
    } else {
        return nullptr;
    }
}

const GrFragmentProcessor* GrContext::createUPMToPMEffect(GrTexture* texture,
                                                          bool swapRAndB,
                                                          const SkMatrix& matrix) const {
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    GrConfigConversionEffect::PMConversion upmToPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != upmToPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, upmToPM, matrix);
    } else {
        return nullptr;
    }
}

bool GrContext::didFailPMUPMConversionTest() const {
    // We should have already called this->testPMConversionsIfNecessary().
    SkASSERT(fDidTestPMConversions);
    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return GrConfigConversionEffect::kNone_PMConversion == fPMToUPMConversion;
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::getResourceCacheLimits(int* maxTextures, size_t* maxTextureBytes) const {
    if (maxTextures) {
        *maxTextures = fResourceCache->getMaxResourceCount();
    }
    if (maxTextureBytes) {
        *maxTextureBytes = fResourceCache->getMaxResourceBytes();
    }
}

void GrContext::setResourceCacheLimits(int maxTextures, size_t maxTextureBytes) {
    fResourceCache->setLimits(maxTextures, maxTextureBytes);
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
}
