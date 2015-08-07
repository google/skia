
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

#include "GrAARectRenderer.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPipeline.h"
#include "GrMemoryPool.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrVertexBuffer.h"

#include "batches/GrBatch.h"
#include "batches/GrRectBatch.h"

#include "SkStrokeRec.h"

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::GrDrawTarget(GrGpu* gpu, GrResourceProvider* resourceProvider)
    : fGpu(SkRef(gpu))
    , fCaps(SkRef(gpu->caps()))
    , fResourceProvider(resourceProvider)
    , fGpuTraceMarkerCount(0)
    , fFlushing(false) {
}

GrDrawTarget::~GrDrawTarget() {
    fGpu->unref();
    fCaps->unref();
}

////////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::setupDstReadIfNecessary(const GrPipelineBuilder& pipelineBuilder,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           GrXferProcessor::DstTexture* dstTexture,
                                           const SkRect* drawBounds) {
    if (!pipelineBuilder.willXPNeedDstTexture(*this->caps(), colorPOI, coveragePOI)) {
        return true;
    }

    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();

    if (this->caps()->textureBarrierSupport()) {
        if (GrTexture* rtTex = rt->asTexture()) {
            // The render target is a texture, so we can read from it directly in the shader. The XP
            // will be responsible to detect this situation and request a texture barrier.
            dstTexture->setTexture(rtTex);
            dstTexture->setOffset(0, 0);
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
            GrCapsDebugf(fCaps, "Missed an early reject. "
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

    static const uint32_t kFlags = 0;
    SkAutoTUnref<GrTexture> copy(fResourceProvider->createApproxTexture(desc, kFlags));

    if (!copy) {
        SkDebugf("Failed to create temporary copy of destination texture.\n");
        return false;
    }
    SkIPoint dstPoint = {0, 0};
    this->copySurface(copy, rt, copyRect, dstPoint);
    dstTexture->setTexture(copy);
    dstTexture->setOffset(copyRect.fLeft, copyRect.fTop);
    return true;
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

void GrDrawTarget::drawBatch(const GrPipelineBuilder& pipelineBuilder, GrBatch* batch) {
    // TODO some kind of checkdraw, but not at this level

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, &batch->bounds())) {
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

void GrDrawTarget::stencilPath(const GrPipelineBuilder& pipelineBuilder,
                               const GrPathProcessor* pathProc,
                               const GrPath* path,
                               GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    this->onStencilPath(pipelineBuilder, pathProc, path, scissorState, stencilSettings);
}

void GrDrawTarget::drawPath(const GrPipelineBuilder& pipelineBuilder,
                            const GrPathProcessor* pathProc,
                            const GrPath* path,
                            GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path rendering.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    SkRect devBounds = path->getBounds();
    pathProc->viewMatrix().mapRect(&devBounds);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, &devBounds)) {
       return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    GrDrawTarget::PipelineInfo pipelineInfo(pipelineBuilder, &scissorState, pathProc, &devBounds,
                                            this);
    if (pipelineInfo.mustSkipDraw()) {
        return;
    }

    this->onDrawPath(pathProc, path, stencilSettings, pipelineInfo);
}

void GrDrawTarget::drawPaths(const GrPipelineBuilder& pipelineBuilder,
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
    SkASSERT(0 == reinterpret_cast<intptr_t>(indices) %
             GrPathRange::PathIndexSizeInBytes(indexType));
    SkASSERT(transformValues);

    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, NULL)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
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

void GrDrawTarget::drawBWRect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkRect* localRect,
                              const SkMatrix* localMatrix) {
   SkAutoTUnref<GrBatch> batch(GrRectBatch::Create(color, viewMatrix, rect, localRect,
                                                   localMatrix));
   this->drawBatch(pipelineBuilder, batch);
}

void GrDrawTarget::drawAARect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkRect& devRect) {
   GrAARectRenderer::FillAARect(this, pipelineBuilder, color, viewMatrix, rect, devRect);
}

void GrDrawTarget::clear(const SkIRect* rect,
                         GrColor color,
                         bool canIgnoreRect,
                         GrRenderTarget* renderTarget) {
    SkIRect rtRect = SkIRect::MakeWH(renderTarget->width(), renderTarget->height());
    SkIRect clippedRect;
    if (!rect ||
        (canIgnoreRect && this->caps()->fullClearIsFree()) ||
        rect->contains(rtRect)) {
        rect = &rtRect;
    } else {
        clippedRect = *rect;
        if (!clippedRect.intersect(rtRect)) {
            return;
        }
        rect = &clippedRect;
    }

    if (fCaps->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        if (rect == &rtRect) {
            this->discard(renderTarget);
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(renderTarget);

        this->drawSimpleRect(pipelineBuilder, color, SkMatrix::I(), *rect);
    } else {       
        this->onClear(*rect, color, renderTarget);
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

void GrDrawTarget::copySurface(GrSurface* dst,
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
        return;
    }

    this->onCopySurface(dst, src, clippedSrcRect, clippedDstPoint);
}

const GrPipeline* GrDrawTarget::setupPipeline(const PipelineInfo& pipelineInfo,
                                              void* pipelineAddr) {
    return SkNEW_PLACEMENT_ARGS(pipelineAddr, GrPipeline, (*pipelineInfo.fPipelineBuilder,
                                                            pipelineInfo.fColorPOI,
                                                            pipelineInfo.fCoveragePOI,
                                                            *this->caps(),
                                                            *pipelineInfo.fScissor,
                                                            &pipelineInfo.fDstTexture));
}
///////////////////////////////////////////////////////////////////////////////

GrDrawTarget::PipelineInfo::PipelineInfo(const GrPipelineBuilder& pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrPrimitiveProcessor* primProc,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(&pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(primProc);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(primProc);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstTexture, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

GrDrawTarget::PipelineInfo::PipelineInfo(const GrPipelineBuilder& pipelineBuilder,
                                         GrScissorState* scissor,
                                         const GrBatch* batch,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target)
    : fPipelineBuilder(&pipelineBuilder)
    , fScissor(scissor) {
    fColorPOI = fPipelineBuilder->colorProcInfo(batch);
    fCoveragePOI = fPipelineBuilder->coverageProcInfo(batch);
    if (!target->setupDstReadIfNecessary(*fPipelineBuilder, fColorPOI, fCoveragePOI,
                                         &fDstTexture, devBounds)) {
        fPipelineBuilder = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
GrClipTarget::GrClipTarget(GrContext* context)
    : INHERITED(context->getGpu(), context->resourceProvider())
    , fContext(context) {
    fClipMaskManager.reset(SkNEW_ARGS(GrClipMaskManager, (this)));
}


bool GrClipTarget::setupClip(const GrPipelineBuilder& pipelineBuilder,
                             GrPipelineBuilder::AutoRestoreFragmentProcessorState* arfps,
                             GrPipelineBuilder::AutoRestoreStencil* ars,
                             GrScissorState* scissorState,
                             const SkRect* devBounds) {
    return fClipMaskManager->setupClipping(pipelineBuilder,
                                           arfps,
                                           ars,
                                           scissorState,
                                           devBounds);
}

void GrClipTarget::purgeResources() {
    // The clip mask manager can rebuild all its clip masks so just
    // get rid of them all.
    fClipMaskManager->purgeResources();
};
