
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
    , fFlushing(false)
    , fLastFlushToken(0) {
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
                                           const SkRect& batchBounds) {
    SkRect bounds = batchBounds;
    bounds.outset(0.5f, 0.5f);

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

    SkIRect drawIBounds;
    bounds.roundOut(&drawIBounds);
    if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
        GrCapsDebugf(fCaps, "Missed an early reject. "
                        "Bailing on draw from setupDstReadIfNecessary.\n");
#endif
        return false;
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

    GrBatchFlushState flushState(fGpu, fResourceProvider, fLastFlushToken);

    // Loop over all batches and generate geometry
    for (int i = 0; i < fBatches.count(); ++i) {
        fBatches[i]->prepare(&flushState);
    }

    // Upload all data to the GPU
    flushState.preIssueDraws();

    // Draw all the generated geometry.
    for (int i = 0; i < fBatches.count(); ++i) {
        fBatches[i]->draw(&flushState);
    }

    fLastFlushToken = flushState.lastFlushedToken();

    fFlushing = false;
    this->reset();
}

void GrDrawTarget::reset() {
    fBatches.reset();
}

void GrDrawTarget::drawBatch(const GrPipelineBuilder& pipelineBuilder, GrDrawBatch* batch) {
    // Setup clip
    GrScissorState scissorState;
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    GrPipelineBuilder::AutoRestoreStencil ars;
    if (!this->setupClip(pipelineBuilder, &arfps, &ars, &scissorState, &batch->bounds())) {
        return;
    }

    GrPipeline::CreateArgs args;
    if (!this->installPipelineInDrawBatch(&pipelineBuilder, &scissorState, batch)) {
        return;
    }

    this->recordBatch(batch);
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
                               const SkMatrix& viewMatrix,
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

    GrBatch* batch = GrStencilPathBatch::Create(viewMatrix,
                                                pipelineBuilder.isHWAntialias(),
                                                stencilSettings, scissorState,
                                                pipelineBuilder.getRenderTarget(),
                                                path);
    this->recordBatch(batch);
    batch->unref();
}

void GrDrawTarget::drawPath(const GrPipelineBuilder& pipelineBuilder,
                            const SkMatrix& viewMatrix,
                            GrColor color,
                            const GrPath* path,
                            GrPathRendering::FillType fill) {
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    GrDrawPathBatchBase* batch = GrDrawPathBatch::Create(viewMatrix, color, path);
    this->drawPathBatch(pipelineBuilder, batch, fill);
    batch->unref();
}

void GrDrawTarget::drawPathsFromRange(const GrPipelineBuilder& pipelineBuilder,
                                      const SkMatrix& viewMatrix,
                                      const SkMatrix& localMatrix,
                                      GrColor color,
                                      GrPathRangeDraw* draw,
                                      GrPathRendering::FillType fill) {
    GrDrawPathBatchBase* batch = GrDrawPathRangeBatch::Create(viewMatrix, localMatrix, color, draw);
    this->drawPathBatch(pipelineBuilder, batch, fill);
    batch->unref();
}

void GrDrawTarget::drawPathBatch(const GrPipelineBuilder& pipelineBuilder,
                                 GrDrawPathBatchBase* batch,
                                 GrPathRendering::FillType fill) {
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

    GrPipeline::CreateArgs args;
    if (!this->installPipelineInDrawBatch(&pipelineBuilder, &scissorState, batch)) {
        return;
    }

    this->recordBatch(batch);
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
        this->recordBatch(batch);
        batch->unref();
    }
}

void GrDrawTarget::discard(GrRenderTarget* renderTarget) {
    if (this->caps()->discardRenderTargetSupport()) {
        GrBatch* batch = new GrDiscardBatch(renderTarget);
        this->recordBatch(batch);
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
        this->recordBatch(batch);
        batch->unref();
    }
}

template <class Left, class Right> static bool intersect(const Left& a, const Right& b) {
    SkASSERT(a.fLeft <= a.fRight && a.fTop <= a.fBottom &&
             b.fLeft <= b.fRight && b.fTop <= b.fBottom);
    return a.fLeft < b.fRight && b.fLeft < a.fRight && a.fTop < b.fBottom && b.fTop < a.fBottom;
}

void GrDrawTarget::recordBatch(GrBatch* batch) {
    // Check if there is a Batch Draw we can batch with by linearly searching back until we either
    // 1) check every draw
    // 2) intersect with something
    // 3) find a 'blocker'
    // Experimentally we have found that most batching occurs within the first 10 comparisons.
    static const int kMaxLookback = 10;

    GrBATCH_INFO("Re-Recording (%s, B%u)\n"
        "\tBounds (%f, %f, %f, %f)\n",
        batch->name(),
        batch->uniqueID(),
        batch->bounds().fLeft, batch->bounds().fRight,
        batch->bounds().fTop, batch->bounds().fBottom);
    GrBATCH_INFO(SkTabString(batch->dumpInfo(), 1).c_str());
    GrBATCH_INFO("\tOutcome:\n");    
    int maxCandidates = SkTMin(kMaxLookback, fBatches.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            GrBatch* candidate = fBatches.fromBack(i);
            // We cannot continue to search backwards if the render target changes
            if (candidate->renderTargetUniqueID() != batch->renderTargetUniqueID()) {
                GrBATCH_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                    candidate->name(), candidate->uniqueID());
                break;
            }
            if (candidate->combineIfPossible(batch, *this->caps())) {
                GrBATCH_INFO("\t\tCombining with (%s, B%u)\n", candidate->name(),
                    candidate->uniqueID());
                return;
            }
            // Stop going backwards if we would cause a painter's order violation.
            if (intersect(candidate->bounds(), batch->bounds())) {
                GrBATCH_INFO("\t\tIntersects with (%s, B%u)\n", candidate->name(),
                    candidate->uniqueID());
                break;
            }
            ++i;
            if (i == maxCandidates) {
                GrBATCH_INFO("\t\tReached max lookback or beginning of batch array %d\n", i);
                break;
            }
        }
    } else {
        GrBATCH_INFO("\t\tFirstBatch\n");
    }
    fBatches.push_back().reset(SkRef(batch));
}

///////////////////////////////////////////////////////////////////////////////

bool GrDrawTarget::installPipelineInDrawBatch(const GrPipelineBuilder* pipelineBuilder,
                                              const GrScissorState* scissor,
                                              GrDrawBatch* batch) {
    GrPipeline::CreateArgs args;
    args.fPipelineBuilder = pipelineBuilder;
    args.fCaps = this->caps();
    args.fScissor = scissor;
    args.fColorPOI = pipelineBuilder->colorProcInfo(batch);
    args.fCoveragePOI = pipelineBuilder->coverageProcInfo(batch);
    if (!this->setupDstReadIfNecessary(*pipelineBuilder, args.fColorPOI,
                                       args.fCoveragePOI, &args.fDstTexture,
                                       batch->bounds())) {
        return false;
    }

    if (!batch->installPipeline(args)) {
        return false;
    }

    return true;
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
    this->recordBatch(batch);
    batch->unref();
}
