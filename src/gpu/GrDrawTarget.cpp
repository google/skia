
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

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

#include "batches/GrClearBatch.h"
#include "batches/GrCopySurfaceBatch.h"
#include "batches/GrDiscardBatch.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrDrawPathBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrStencilPathBatch.h"

#include "SkStrokeRec.h"

////////////////////////////////////////////////////////////////////////////////

GrDrawTarget::GrDrawTarget(GrGpu* gpu, GrResourceProvider* resourceProvider)
    : fGpu(SkRef(gpu))
    , fCaps(SkRef(gpu->caps()))
    , fResourceProvider(resourceProvider)
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

    this->onFlush();

    fFlushing = false;
    this->reset();
}

void GrDrawTarget::drawBatch(const GrPipelineBuilder& pipelineBuilder, GrDrawBatch* batch) {
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

    GrDrawTarget::PipelineInfo pipelineInfo(&pipelineBuilder, &scissorState, batch, &bounds,
                                            this);

    if (!pipelineInfo.valid()) {
        return;
    }
    if (!batch->installPipeline(pipelineInfo.pipelineCreateArgs())) {
        return;
    }
    this->onDrawBatch(batch);
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
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, nullptr)) {
        return;
    }

    // set stencil settings for path
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);

    GrBatch* batch = GrStencilPathBatch::Create(pathProc->viewMatrix(),
                                                pipelineBuilder.isHWAntialias(),
                                                stencilSettings, scissorState,
                                                pipelineBuilder.getRenderTarget(),
                                                path);
    this->onDrawBatch(batch);
    batch->unref();
}

void GrDrawTarget::drawPath(const GrPipelineBuilder& pipelineBuilder,
                            const GrPathProcessor* pathProc,
                            const GrPath* path,
                            GrPathRendering::FillType fill) {
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    GrDrawPathBatch* batch = GrDrawPathBatch::Create(pathProc, path);

    // This looks like drawBatch() but there is an added wrinkle that stencil settings get inserted
    // after setupClip() but before onDrawBatch(). TODO: Figure out a better model for handling
    // stencil settings WRT interactions between pipeline(builder), clipmaskmanager, and batches.

    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, &batch->bounds())) {
        return;
    }

    // Ensure the render target has a stencil buffer and get the stencil settings.
    GrStencilSettings stencilSettings;
    GrRenderTarget* rt = pipelineBuilder.getRenderTarget();
    GrStencilAttachment* sb = rt->renderTargetPriv().attachStencilAttachment();
    this->getPathStencilSettingsForFilltype(fill, sb, &stencilSettings);
    batch->setStencilSettings(stencilSettings);

    // Don't compute a bounding box for dst copy texture, we'll opt
    // instead for it to just copy the entire dst. Realistically this is a moot
    // point, because any context that supports NV_path_rendering will also
    // support NV_blend_equation_advanced.
    GrDrawTarget::PipelineInfo pipelineInfo(&pipelineBuilder, &scissorState, batch, nullptr, this);

    if (!pipelineInfo.valid()) {
        return;
    }
    if (!batch->installPipeline(pipelineInfo.pipelineCreateArgs())) {
        return;
    }

    this->onDrawBatch(batch);
    batch->unref();
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
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, nullptr)) {
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
    GrDrawTarget::PipelineInfo pipelineInfo(&pipelineBuilder, &scissorState, pathProc, nullptr, this);
    if (!pipelineInfo.valid()) {
        return;
    }

    this->onDrawPaths(pathProc, pathRange, indices, indexType, transformValues,
                      transformType, count, stencilSettings, pipelineInfo);
}

void GrDrawTarget::drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect) {
   SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect,
                                                                       nullptr, nullptr));
   this->drawBatch(pipelineBuilder, batch);
}

void GrDrawTarget::drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkMatrix& localMatrix) {
   SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect,
                                                                       nullptr, &localMatrix));
   this->drawBatch(pipelineBuilder, batch);
}

void GrDrawTarget::drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkRect& localRect) {
   SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect,
                                                                       &localRect, nullptr));
   this->drawBatch(pipelineBuilder, batch);
}


void GrDrawTarget::drawAARect(const GrPipelineBuilder& pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkRect& devRect) {
    SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateAAFill(color, viewMatrix, rect,
                                                                     devRect));
    this->drawBatch(pipelineBuilder, batch);
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

        this->drawNonAARect(pipelineBuilder, color, SkMatrix::I(), *rect);
    } else {
        GrBatch* batch = new GrClearBatch(*rect, color, renderTarget);
        this->onDrawBatch(batch);
        batch->unref();
    }
}

void GrDrawTarget::discard(GrRenderTarget* renderTarget) {
    if (this->caps()->discardRenderTargetSupport()) {
        GrBatch* batch = new GrDiscardBatch(renderTarget);
        this->onDrawBatch(batch);
        batch->unref();
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::copySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) {
    GrBatch* batch = GrCopySurfaceBatch::Create(dst, src, srcRect, dstPoint);
    if (batch) {
        this->onDrawBatch(batch);
        batch->unref();
    }
}

///////////////////////////////////////////////////////////////////////////////

GrDrawTarget::PipelineInfo::PipelineInfo(const GrPipelineBuilder* pipelineBuilder,
                                         const GrScissorState* scissor,
                                         const GrPrimitiveProcessor* primProc,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target) {
    fArgs.fPipelineBuilder = pipelineBuilder;
    fArgs.fCaps = target->caps();
    fArgs.fScissor = scissor;
    fArgs.fColorPOI = fArgs.fPipelineBuilder->colorProcInfo(primProc);
    fArgs.fCoveragePOI = fArgs.fPipelineBuilder->coverageProcInfo(primProc);
    if (!target->setupDstReadIfNecessary(*fArgs.fPipelineBuilder, fArgs.fColorPOI,
                                         fArgs.fCoveragePOI, &fArgs.fDstTexture, devBounds)) {
        fArgs.fPipelineBuilder = nullptr;
    }
}

GrDrawTarget::PipelineInfo::PipelineInfo(const GrPipelineBuilder* pipelineBuilder,
                                         const GrScissorState* scissor,
                                         const GrDrawBatch* batch,
                                         const SkRect* devBounds,
                                         GrDrawTarget* target) {
    fArgs.fPipelineBuilder = pipelineBuilder;
    fArgs.fCaps = target->caps();
    fArgs.fScissor = scissor;
    fArgs.fColorPOI = fArgs.fPipelineBuilder->colorProcInfo(batch);
    fArgs.fCoveragePOI = fArgs.fPipelineBuilder->coverageProcInfo(batch);
    if (!target->setupDstReadIfNecessary(*fArgs.fPipelineBuilder, fArgs.fColorPOI,
                                         fArgs.fCoveragePOI, &fArgs.fDstTexture, devBounds)) {
        fArgs.fPipelineBuilder = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
GrClipTarget::GrClipTarget(GrContext* context)
    : INHERITED(context->getGpu(), context->resourceProvider())
    , fContext(context) {
    fClipMaskManager.reset(new GrClipMaskManager(this));
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

void GrClipTarget::clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* rt) {
    GrBatch* batch = new GrClearStencilClipBatch(rect, insideClip, rt);
    this->onDrawBatch(batch);
    batch->unref();
}
