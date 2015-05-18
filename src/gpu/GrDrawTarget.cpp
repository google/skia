
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

#include "GrBatch.h"
#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrPath.h"
#include "GrPipeline.h"
#include "GrMemoryPool.h"
#include "GrRectBatch.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfacePriv.h"
#include "GrTemplates.h"
#include "GrTexture.h"
#include "GrVertexBuffer.h"

#include "SkStrokeRec.h"

////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INVAL_BUFFER 0xdeadcafe
#define DEBUG_INVAL_START_IDX -1

GrDrawTarget::GrDrawTarget(GrContext* context)
    : fContext(context)
    , fCaps(SkRef(context->getGpu()->caps()))
    , fGpuTraceMarkerCount(0)
    , fFlushing(false) {
    SkASSERT(context);
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::setupDstReadIfNecessary(const GrPipelineBuilder& pipelineBuilder,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           GrDeviceCoordTexture* dstCopy,
                                           const SkRect* drawBounds) {
    if (!pipelineBuilder.willXPNeedDstCopy(*this->caps(), colorPOI, coveragePOI)) {
        return true;
    }

    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();

    if (this->caps()->textureBarrierSupport()) {
        if (GrTexture* rtTex = rt->asTexture()) {
            // The render target is a texture, se we can read from it directly in the shader. The XP
            // will be responsible to detect this situation and request a texture barrier.
            dstCopy->setTexture(rtTex);
            dstCopy->setOffset(0, 0);
            return true;
        }
    }

    SkIRect copyRect;
    pipelineBuilder.clip().getConservativeBounds(rt, &copyRect);

    if (drawBounds) {
        SkIRect drawIBounds;
        drawBounds->roundOut(&drawIBounds);
        if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
            GrContextDebugf(fContext, "Missed an early reject. "
                                      "Bailing on draw from setupDstReadIfNecessary.\n");
#endif
            return false;
        }
    } else {
#ifdef SK_DEBUG
        //SkDebugf("No dev bounds when dst copy is made.\n");
#endif
    }

    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrSurfaceDesc desc;
    if (!this->getGpu()->initCopySurfaceDstDesc(rt, &desc)) {
        desc.fOrigin = kDefault_GrSurfaceOrigin;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fConfig = rt->config();
    }


    desc.fWidth = copyRect.width();
    desc.fHeight = copyRect.height();

    SkAutoTUnref<GrTexture> copy(fContext->textureProvider()->refScratchTexture(desc,
        GrTextureProvider::kApprox_ScratchTexMatch));

    if (!copy) {
        SkDebugf("Failed to create temporary copy of destination texture.\n");
        return false;
    }
    SkIPoint dstPoint = {0, 0};
    if (this->copySurface(copy, rt, copyRect, dstPoint)) {
        dstCopy->setTexture(copy);
        dstCopy->setOffset(copyRect.fLeft, copyRect.fTop);
        return true;
    } else {
        return false;
    }
}

void GrDrawTarget::flush() {
    if (fFlushing) {
        return;
    }
    fFlushing = true;

    this->getGpu()->saveActiveTraceMarkers();

    this->onFlush();

    this->getGpu()->restoreActiveTraceMarkers();

    fFlushing = false;
    this->reset();
}

void GrDrawTarget::drawBatch(GrPipelineBuilder* pipelineBuilder,
                             GrBatch* batch) {
    SkASSERT(pipelineBuilder);
    // TODO some kind of checkdraw, but not at this level

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, &batch->bounds())) {
        return;
    }

    // Batch bounds are tight, so for dev copies
    // TODO move this into setupDstReadIfNecessary when paths are in batch
    SkRect bounds = batch->bounds();
    bounds.outset(0.5f, 0.5f);

    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, batch, &bounds,
                                            this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawBatch(batch, pipelineInfo);
}

static const GrStencilSettings& winding_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kIncClamp_StencilOp,
        kIncClamp_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

static const GrStencilSettings& even_odd_path_stencil_settings() {
    GR_STATIC_CONST_SAME_STENCIL_STRUCT(gSettings,
        kInvert_StencilOp,
        kInvert_StencilOp,
        kAlwaysIfInClip_StencilFunc,
        0xFFFF, 0xFFFF, 0xFFFF);
    return *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&gSettings);
}

void GrDrawTarget::getPathStencilSettingsForFilltype(GrPathRendering::FillType fill,
                                                     const GrStencilAttachment* sb,
                                                     GrStencilSettings* outStencilSettings) {

    switch (fill) {
        default:
            SkFAIL("Unexpected path fill.");
        case GrPathRendering::kWinding_FillType:
            *outStencilSettings = winding_path_stencil_settings();
            break;
        case GrPathRendering::kEvenOdd_FillType:
            *outStencilSettings = even_odd_path_stencil_settings();
            break;
    }
    this->clipMaskManager()->adjustPathStencilParams(sb, outStencilSettings);
}

void GrDrawTarget::stencilPath(GrPipelineBuilder* pipelineBuilder,
                               const GrPathProcessor* pathProc,
                               const GrPath* path,
                               GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());
    SkASSERT(pipelineBuilder);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    this->onStencilPath(*pipelineBuilder, pathProc, path, scissorState, stencilSettings);
}

void GrDrawTarget::drawPath(GrPipelineBuilder* pipelineBuilder,
                            const GrPathProcessor* pathProc,
                            const GrPath* path,
                            GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path rendering.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());
    SkASSERT(pipelineBuilder);

    SkRect devBounds = path->getBounds();
    pathProc->viewMatrix().mapRect(&devBounds);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, &devBounds)) {
       return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, pathProc, &devBounds,
                                            this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawPath(pathProc, path, stencilSettings, pipelineInfo);
}

void GrDrawTarget::drawPaths(GrPipelineBuilder* pipelineBuilder,
                             const GrPathProcessor* pathProc,
                             const GrPathRange* pathRange,
                             const void* indices,
                             PathIndexType indexType,
                             const float transformValues[],
                             PathTransformType transformType,
                             int count,
                             GrPathRendering::FillType fill) {
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(0 == reinterpret_cast<long>(indices) % GrPathRange::PathIndexSizeInBytes(indexType));
    SkASSERT(transformValues);
    SkASSERT(pipelineBuilder);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
    GrPipelineBuilder::AutoRestoreStencil ars;

    if (!this->setupClip(pipelineBuilder, &arfp, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    // Don't compute a bounding box for dst copy texture, we'll opt
    // instead for it to just copy the entire dst. Realistically this is a moot
    // point, because any context that supports NV_path_rendering will also
    // support NV_blend_equation_advanced.
    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, pathProc, NULL, this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawPaths(pathProc, pathRange, indices, indexType, transformValues,
                      transformType, count, stencilSettings, pipelineInfo);
}

void GrDrawTarget::drawRect(GrPipelineBuilder* pipelineBuilder,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) {
   SkAutoTUnref<GrBatch> batch(GrRectBatch::Create(color, viewMatrix, rect, localRect,
                                                   localMatrix));
   this->drawBatch(pipelineBuilder, batch);
}

void GrDrawTarget::clear(const SkIRect* rect,
                         GrColor color,
                         bool canIgnoreRect,
                         GrRenderTarget* renderTarget) {
    if (fCaps->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        SkIRect rtRect = SkIRect::MakeWH(renderTarget->width(), renderTarget->height());
        if (NULL == rect || canIgnoreRect || rect->contains(rtRect)) {
            rect = &rtRect;
            // We first issue a discard() since that may help tilers.
            this->discard(renderTarget);
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(renderTarget);

        this->drawSimpleRect(&pipelineBuilder, color, SkMatrix::I(), *rect);
    } else {       
        this->onClear(rect, color, canIgnoreRect, renderTarget);
    }
}

typedef GrTraceMarkerSet::Iter TMIter;
void GrDrawTarget::saveActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fStoredTraceMarkers.count());
        fStoredTraceMarkers.addSet(fActiveTraceMarkers);
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->removeGpuTraceMarker(&(*iter));
        }
    }
}

void GrDrawTarget::restoreActiveTraceMarkers() {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(0 == fActiveTraceMarkers.count());
        for (TMIter iter = fStoredTraceMarkers.begin(); iter != fStoredTraceMarkers.end(); ++iter) {
            this->addGpuTraceMarker(&(*iter));
        }
        for (TMIter iter = fActiveTraceMarkers.begin(); iter != fActiveTraceMarkers.end(); ++iter) {
            this->fStoredTraceMarkers.remove(*iter);
        }
    }
}

void GrDrawTarget::addGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 0);
        this->fActiveTraceMarkers.add(*marker);
        ++fGpuTraceMarkerCount;
    }
}

void GrDrawTarget::removeGpuTraceMarker(const GrGpuTraceMarker* marker) {
    if (this->caps()->gpuTracingSupport()) {
        SkASSERT(fGpuTraceMarkerCount >= 1);
        this->fActiveTraceMarkers.remove(*marker);
        --fGpuTraceMarkerCount;
    }
}

////////////////////////////////////////////////////////////////////////////////

namespace {
// returns true if the read/written rect intersects the src/dst and false if not.
bool clip_srcrect_and_dstpoint(const GrSurface* dst,
                               const GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint,
                               SkIRect* clippedSrcRect,
                               SkIPoint* clippedDstPoint) {
    *clippedSrcRect = srcRect;
    *clippedDstPoint = dstPoint;

    // clip the left edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fLeft < 0) {
        clippedDstPoint->fX -= clippedSrcRect->fLeft;
        clippedSrcRect->fLeft = 0;
    }
    if (clippedDstPoint->fX < 0) {
        clippedSrcRect->fLeft -= clippedDstPoint->fX;
        clippedDstPoint->fX = 0;
    }

    // clip the top edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fTop < 0) {
        clippedDstPoint->fY -= clippedSrcRect->fTop;
        clippedSrcRect->fTop = 0;
    }
    if (clippedDstPoint->fY < 0) {
        clippedSrcRect->fTop -= clippedDstPoint->fY;
        clippedDstPoint->fY = 0;
    }

    // clip the right edge to the src and dst bounds.
    if (clippedSrcRect->fRight > src->width()) {
        clippedSrcRect->fRight = src->width();
    }
    if (clippedDstPoint->fX + clippedSrcRect->width() > dst->width()) {
        clippedSrcRect->fRight = clippedSrcRect->fLeft + dst->width() - clippedDstPoint->fX;
    }

    // clip the bottom edge to the src and dst bounds.
    if (clippedSrcRect->fBottom > src->height()) {
        clippedSrcRect->fBottom = src->height();
    }
    if (clippedDstPoint->fY + clippedSrcRect->height() > dst->height()) {
        clippedSrcRect->fBottom = clippedSrcRect->fTop + dst->height() - clippedDstPoint->fY;
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    return !clippedSrcRect->isEmpty();
}
}

bool GrDrawTarget::copySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src);

    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the src or dst then we've already succeeded.
    if (!clip_srcrect_and_dstpoint(dst,
                                   src,
                                   srcRect,
                                   dstPoint,
                                   &clippedSrcRect,
                                   &clippedDstPoint)) {
        return true;
    }

    if (this->getGpu()->canCopySurface(dst, src, clippedSrcRect, clippedDstPoint)) {
        this->onCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
        return true;
    }

    GrRenderTarget* rt = dst->asRenderTarget();
    GrTexture* tex = src->asTexture();

    if ((dst == src) || !rt || !tex) {
        return false;
    }

    GrPipelineBuilder pipelineBuilder;
    pipelineBuilder.setRenderTarget(rt);
    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(clippedSrcRect.fLeft - clippedDstPoint.fX),
                        SkIntToScalar(clippedSrcRect.fTop - clippedDstPoint.fY));
    matrix.postIDiv(tex->width(), tex->height());
    pipelineBuilder.addColorTextureProcessor(tex, matrix);
    SkIRect dstRect = SkIRect::MakeXYWH(clippedDstPoint.fX,
                                        clippedDstPoint.fY,
                                        clippedSrcRect.width(),
                                        clippedSrcRect.height());
    this->drawSimpleRect(&pipelineBuilder, GrColor_WHITE, SkMatrix::I(), dstRect);
    return true;
}

bool GrDrawTarget::canCopySurface(const GrSurface* dst,
                                  const GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) {
    SkASSERT(dst);
    SkASSERT(src);

    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the src or dst then we're guaranteed success
    if (!clip_srcrect_and_dstpoint(dst,
                                   src,
                                   srcRect,
                                   dstPoint,
                                   &clippedSrcRect,
                                   &clippedDstPoint)) {
        return true;
    }
    return ((dst != src) && dst->asRenderTarget() && src->asTexture()) ||
           this->getGpu()->canCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
}

void GrDrawTarget::setupPipeline(const PipelineInfo& pipelineInfo,
                                 GrPipeline* pipeline) {
    SkNEW_PLACEMENT_ARGS(pipeline, GrPipeline, (*pipelineInfo.fPipelineBuilder,
                                                pipelineInfo.fColorPOI,
                                                pipelineInfo.fCoveragePOI,
                                                *this->caps(),
                                                *pipelineInfo.fScissor,
                                                &pipelineInfo.fDstCopy));
}
///////////////////////////////////////////////////////////////////////////////

GrDrawTarget::PipelineInfo::PipelineInfo(GrPipelineBuilder* pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrPrimitiveProcessor* primProc,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(primProc);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(primProc);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstCopy, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

GrDrawTarget::PipelineInfo::PipelineInfo(GrPipelineBuilder* pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrBatch* batch,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(batch);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(batch);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstCopy, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrShaderCaps::reset() {
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fDualSourceBlendingSupport = false;

    fShaderPrecisionVaries = false;
}

GrShaderCaps& GrShaderCaps::operator=(const GrShaderCaps& other) {
    fShaderDerivativeSupport = other.fShaderDerivativeSupport;
    fGeometryShaderSupport = other.fGeometryShaderSupport;
    fPathRenderingSupport = other.fPathRenderingSupport;
    fDstReadInShaderSupport = other.fDstReadInShaderSupport;
    fDualSourceBlendingSupport = other.fDualSourceBlendingSupport;

    fShaderPrecisionVaries = other.fShaderPrecisionVaries;
    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            fFloatPrecisions[s][p] = other.fFloatPrecisions[s][p];
        }
    }
    return *this;
}

static const char* shader_type_to_string(GrShaderType type) {
    switch (type) {
    case kVertex_GrShaderType:
        return "vertex";
    case kGeometry_GrShaderType:
        return "geometry";
    case kFragment_GrShaderType:
        return "fragment";
    }
    return "";
}

static const char* precision_to_string(GrSLPrecision p) {
    switch (p) {
    case kLow_GrSLPrecision:
        return "low";
    case kMedium_GrSLPrecision:
        return "medium";
    case kHigh_GrSLPrecision:
        return "high";
    }
    return "";
}

SkString GrShaderCaps::dump() const {
    SkString r;
    static const char* gNY[] = { "NO", "YES" };
    r.appendf("Shader Derivative Support          : %s\n", gNY[fShaderDerivativeSupport]);
    r.appendf("Geometry Shader Support            : %s\n", gNY[fGeometryShaderSupport]);
    r.appendf("Path Rendering Support             : %s\n", gNY[fPathRenderingSupport]);
    r.appendf("Dst Read In Shader Support         : %s\n", gNY[fDstReadInShaderSupport]);
    r.appendf("Dual Source Blending Support       : %s\n", gNY[fDualSourceBlendingSupport]);

    r.appendf("Shader Float Precisions (varies: %s):\n", gNY[fShaderPrecisionVaries]);

    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        GrShaderType shaderType = static_cast<GrShaderType>(s);
        r.appendf("\t%s:\n", shader_type_to_string(shaderType));
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            if (fFloatPrecisions[s][p].supported()) {
                GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                r.appendf("\t\t%s: log_low: %d log_high: %d bits: %d\n",
                    precision_to_string(precision),
                    fFloatPrecisions[s][p].fLogRangeLow,
                    fFloatPrecisions[s][p].fLogRangeHigh,
                    fFloatPrecisions[s][p].fBits);
            }
        }
    }

    return r;
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawTargetCaps::reset() {
    fMipMapSupport = false;
    fNPOTTextureTileSupport = false;
    fTwoSidedStencilSupport = false;
    fStencilWrapOpsSupport = false;
    fDiscardRenderTargetSupport = false;
    fReuseScratchTextures = true;
    fGpuTracingSupport = false;
    fCompressedTexSubImageSupport = false;
    fOversizedStencilSupport = false;
    fTextureBarrierSupport = false;

    fUseDrawInsteadOfClear = false;

    fBlendEquationSupport = kBasic_BlendEquationSupport;
    fMapBufferFlags = kNone_MapFlags;

    fMaxRenderTargetSize = 0;
    fMaxTextureSize = 0;
    fMaxSampleCount = 0;

    memset(fConfigRenderSupport, 0, sizeof(fConfigRenderSupport));
    memset(fConfigTextureSupport, 0, sizeof(fConfigTextureSupport));
}

GrDrawTargetCaps& GrDrawTargetCaps::operator=(const GrDrawTargetCaps& other) {
    fMipMapSupport = other.fMipMapSupport;
    fNPOTTextureTileSupport = other.fNPOTTextureTileSupport;
    fTwoSidedStencilSupport = other.fTwoSidedStencilSupport;
    fStencilWrapOpsSupport = other.fStencilWrapOpsSupport;
    fDiscardRenderTargetSupport = other.fDiscardRenderTargetSupport;
    fReuseScratchTextures = other.fReuseScratchTextures;
    fGpuTracingSupport = other.fGpuTracingSupport;
    fCompressedTexSubImageSupport = other.fCompressedTexSubImageSupport;
    fOversizedStencilSupport = other.fOversizedStencilSupport;
    fTextureBarrierSupport = other.fTextureBarrierSupport;

    fUseDrawInsteadOfClear = other.fUseDrawInsteadOfClear;

    fBlendEquationSupport = other.fBlendEquationSupport;
    fMapBufferFlags = other.fMapBufferFlags;

    fMaxRenderTargetSize = other.fMaxRenderTargetSize;
    fMaxTextureSize = other.fMaxTextureSize;
    fMaxSampleCount = other.fMaxSampleCount;

    memcpy(fConfigRenderSupport, other.fConfigRenderSupport, sizeof(fConfigRenderSupport));
    memcpy(fConfigTextureSupport, other.fConfigTextureSupport, sizeof(fConfigTextureSupport));

    return *this;
}

static SkString map_flags_to_string(uint32_t flags) {
    SkString str;
    if (GrDrawTargetCaps::kNone_MapFlags == flags) {
        str = "none";
    } else {
        SkASSERT(GrDrawTargetCaps::kCanMap_MapFlag & flags);
        SkDEBUGCODE(flags &= ~GrDrawTargetCaps::kCanMap_MapFlag);
        str = "can_map";

        if (GrDrawTargetCaps::kSubset_MapFlag & flags) {
            str.append(" partial");
        } else {
            str.append(" full");
        }
        SkDEBUGCODE(flags &= ~GrDrawTargetCaps::kSubset_MapFlag);
    }
    SkASSERT(0 == flags); // Make sure we handled all the flags.
    return str;
}

SkString GrDrawTargetCaps::dump() const {
    SkString r;
    static const char* gNY[] = {"NO", "YES"};
    r.appendf("MIP Map Support                    : %s\n", gNY[fMipMapSupport]);
    r.appendf("NPOT Texture Tile Support          : %s\n", gNY[fNPOTTextureTileSupport]);
    r.appendf("Two Sided Stencil Support          : %s\n", gNY[fTwoSidedStencilSupport]);
    r.appendf("Stencil Wrap Ops  Support          : %s\n", gNY[fStencilWrapOpsSupport]);
    r.appendf("Discard Render Target Support      : %s\n", gNY[fDiscardRenderTargetSupport]);
    r.appendf("Reuse Scratch Textures             : %s\n", gNY[fReuseScratchTextures]);
    r.appendf("Gpu Tracing Support                : %s\n", gNY[fGpuTracingSupport]);
    r.appendf("Compressed Update Support          : %s\n", gNY[fCompressedTexSubImageSupport]);
    r.appendf("Oversized Stencil Support          : %s\n", gNY[fOversizedStencilSupport]);
    r.appendf("Texture Barrier Support            : %s\n", gNY[fTextureBarrierSupport]);
    r.appendf("Draw Instead of Clear [workaround] : %s\n", gNY[fUseDrawInsteadOfClear]);

    r.appendf("Max Texture Size                   : %d\n", fMaxTextureSize);
    r.appendf("Max Render Target Size             : %d\n", fMaxRenderTargetSize);
    r.appendf("Max Sample Count                   : %d\n", fMaxSampleCount);

    static const char* kBlendEquationSupportNames[] = {
        "Basic",
        "Advanced",
        "Advanced Coherent",
    };
    GR_STATIC_ASSERT(0 == kBasic_BlendEquationSupport);
    GR_STATIC_ASSERT(1 == kAdvanced_BlendEquationSupport);
    GR_STATIC_ASSERT(2 == kAdvancedCoherent_BlendEquationSupport);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kBlendEquationSupportNames) == kLast_BlendEquationSupport + 1);

    r.appendf("Blend Equation Support             : %s\n",
              kBlendEquationSupportNames[fBlendEquationSupport]);
    r.appendf("Map Buffer Support                 : %s\n",
              map_flags_to_string(fMapBufferFlags).c_str());

    static const char* kConfigNames[] = {
        "Unknown",  // kUnknown_GrPixelConfig
        "Alpha8",   // kAlpha_8_GrPixelConfig,
        "Index8",   // kIndex_8_GrPixelConfig,
        "RGB565",   // kRGB_565_GrPixelConfig,
        "RGBA444",  // kRGBA_4444_GrPixelConfig,
        "RGBA8888", // kRGBA_8888_GrPixelConfig,
        "BGRA8888", // kBGRA_8888_GrPixelConfig,
        "SRGBA8888",// kSRGBA_8888_GrPixelConfig,
        "ETC1",     // kETC1_GrPixelConfig,
        "LATC",     // kLATC_GrPixelConfig,
        "R11EAC",   // kR11_EAC_GrPixelConfig,
        "ASTC12x12",// kASTC_12x12_GrPixelConfig,
        "RGBAFloat",// kRGBA_float_GrPixelConfig
        "AlphaHalf",// kAlpha_half_GrPixelConfig
    };
    GR_STATIC_ASSERT(0  == kUnknown_GrPixelConfig);
    GR_STATIC_ASSERT(1  == kAlpha_8_GrPixelConfig);
    GR_STATIC_ASSERT(2  == kIndex_8_GrPixelConfig);
    GR_STATIC_ASSERT(3  == kRGB_565_GrPixelConfig);
    GR_STATIC_ASSERT(4  == kRGBA_4444_GrPixelConfig);
    GR_STATIC_ASSERT(5  == kRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(6  == kBGRA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(7  == kSRGBA_8888_GrPixelConfig);
    GR_STATIC_ASSERT(8  == kETC1_GrPixelConfig);
    GR_STATIC_ASSERT(9  == kLATC_GrPixelConfig);
    GR_STATIC_ASSERT(10  == kR11_EAC_GrPixelConfig);
    GR_STATIC_ASSERT(11 == kASTC_12x12_GrPixelConfig);
    GR_STATIC_ASSERT(12 == kRGBA_float_GrPixelConfig);
    GR_STATIC_ASSERT(13 == kAlpha_half_GrPixelConfig);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kConfigNames) == kGrPixelConfigCnt);

    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][0]);
    SkASSERT(!fConfigRenderSupport[kUnknown_GrPixelConfig][1]);

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        r.appendf("%s is renderable: %s, with MSAA: %s\n",
                  kConfigNames[i],
                  gNY[fConfigRenderSupport[i][0]],
                  gNY[fConfigRenderSupport[i][1]]);
    }

    SkASSERT(!fConfigTextureSupport[kUnknown_GrPixelConfig]);

    for (size_t i = 1; i < SK_ARRAY_COUNT(kConfigNames); ++i)  {
        r.appendf("%s is uploadable to a texture: %s\n",
                  kConfigNames[i],
                  gNY[fConfigTextureSupport[i]]);
    }

    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool GrClipTarget::setupClip(GrPipelineBuilder* pipelineBuilder,
                             GrPipelineBuilder::AutoRestoreFragmentProcessors* arfp,
                             GrPipelineBuilder::AutoRestoreStencil* ars,
                             GrScissorState* scissorState,
                             const SkRect* devBounds) {
    return fClipMaskManager.setupClipping(pipelineBuilder,
                                          arfp,
                                          ars,
                                          scissorState,
                                          devBounds);
}
