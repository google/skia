/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

#include "GrAuditTrail.h"
#include "GrCaps.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrPath.h"
#include "GrPipeline.h"
#include "GrMemoryPool.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "gl/GrGLRenderTarget.h"

#include "SkStrokeRec.h"

#include "batches/GrClearBatch.h"
#include "batches/GrCopySurfaceBatch.h"
#include "batches/GrDiscardBatch.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrDrawPathBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrStencilPathBatch.h"

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most batching occurs within the first 10 comparisons.
static const int kDefaultMaxBatchLookback  = 10;
static const int kDefaultMaxBatchLookahead = 10;

GrDrawTarget::GrDrawTarget(GrRenderTarget* rt, GrGpu* gpu, GrResourceProvider* resourceProvider,
                           GrAuditTrail* auditTrail, const Options& options)
    : fGpu(SkRef(gpu))
    , fResourceProvider(resourceProvider)
    , fAuditTrail(auditTrail)
    , fFlags(0)
    , fRenderTarget(rt) {
    // TODO: Stop extracting the context (currently needed by GrClipMaskManager)
    fContext = fGpu->getContext();

    fClipBatchToBounds = options.fClipBatchToBounds;
    fDrawBatchBounds = options.fDrawBatchBounds;
    fMaxBatchLookback = (options.fMaxBatchLookback < 0) ? kDefaultMaxBatchLookback :
                                                          options.fMaxBatchLookback;
    fMaxBatchLookahead = (options.fMaxBatchLookahead < 0) ? kDefaultMaxBatchLookahead :
                                                           options.fMaxBatchLookahead;

    rt->setLastDrawTarget(this);

#ifdef SK_DEBUG
    static int debugID = 0;
    fDebugID = debugID++;
#endif
}

GrDrawTarget::~GrDrawTarget() {
    if (fRenderTarget && this == fRenderTarget->getLastDrawTarget()) {
        fRenderTarget->setLastDrawTarget(nullptr);
    }

    fGpu->unref();
}

////////////////////////////////////////////////////////////////////////////////

// Add a GrDrawTarget-based dependency
void GrDrawTarget::addDependency(GrDrawTarget* dependedOn) {
    SkASSERT(!dependedOn->dependsOn(this));  // loops are bad

    if (this->dependsOn(dependedOn)) {
        return;  // don't add duplicate dependencies
    }

    *fDependencies.push() = dependedOn;
}

// Convert from a GrSurface-based dependency to a GrDrawTarget one
void GrDrawTarget::addDependency(GrSurface* dependedOn) {
    if (dependedOn->asRenderTarget() && dependedOn->asRenderTarget()->getLastDrawTarget()) {
        // If it is still receiving dependencies, this DT shouldn't be closed
        SkASSERT(!this->isClosed());

        GrDrawTarget* dt = dependedOn->asRenderTarget()->getLastDrawTarget();
        if (dt == this) {
            // self-read - presumably for dst reads
        } else {
            this->addDependency(dt);

            // Can't make it closed in the self-read case
            dt->makeClosed();
        }
    }
}

#ifdef SK_DEBUG
void GrDrawTarget::dump() const {
    SkDebugf("--------------------------------------------------------------\n");
    SkDebugf("node: %d -> RT: %d\n", fDebugID, fRenderTarget ? fRenderTarget->getUniqueID() : -1);
    SkDebugf("relies On (%d): ", fDependencies.count());
    for (int i = 0; i < fDependencies.count(); ++i) {
        SkDebugf("%d, ", fDependencies[i]->fDebugID);
    }
    SkDebugf("\n");
    SkDebugf("batches (%d):\n", fBatches.count());
    for (int i = 0; i < fBatches.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fBatches[i]) {
            SkDebugf("%d: <combined forward>\n", i);
        } else {
            SkDebugf("%d: %s\n", i, fBatches[i]->name());
            SkString str = fBatches[i]->dumpInfo();
            SkDebugf("%s\n", str.c_str());
        }
    }
}
#endif

bool GrDrawTarget::setupDstReadIfNecessary(const GrPipelineBuilder& pipelineBuilder,
                                           GrRenderTarget* rt,
                                           const GrClip& clip,
                                           const GrPipelineOptimizations& optimizations,
                                           GrXferProcessor::DstTexture* dstTexture,
                                           const SkRect& batchBounds) {
    SkRect bounds = batchBounds;
    bounds.outset(0.5f, 0.5f);

    if (!pipelineBuilder.willXPNeedDstTexture(*this->caps(), optimizations)) {
        return true;
    }

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
    clip.getConservativeBounds(rt->width(), rt->height(), &copyRect);

    SkIRect drawIBounds;
    bounds.roundOut(&drawIBounds);
    if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
        GrCapsDebugf(this->caps(), "Missed an early reject. "
                                   "Bailing on draw from setupDstReadIfNecessary.\n");
#endif
        return false;
    }

    // MSAA consideration: When there is support for reading MSAA samples in the shader we could
    // have per-sample dst values by making the copy multisampled.
    GrSurfaceDesc desc;
    if (!fGpu->initCopySurfaceDstDesc(rt, &desc)) {
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

void GrDrawTarget::prepareBatches(GrBatchFlushState* flushState) {
    // Semi-usually the drawTargets are already closed at this point, but sometimes Ganesh
    // needs to flush mid-draw. In that case, the SkGpuDevice's drawTargets won't be closed
    // but need to be flushed anyway. Closing such drawTargets here will mean new
    // drawTargets will be created to replace them if the SkGpuDevice(s) write to them again.
    this->makeClosed();

    // Loop over the batches that haven't yet generated their geometry
    for (int i = 0; i < fBatches.count(); ++i) {
        if (fBatches[i]) {
            fBatches[i]->prepare(flushState);
        }
    }
}

void GrDrawTarget::drawBatches(GrBatchFlushState* flushState) {
    // Draw all the generated geometry.
    SkRandom random;
    GrRenderTarget* currentRT = nullptr;
    SkAutoTDelete<GrGpuCommandBuffer> commandBuffer;
    for (int i = 0; i < fBatches.count(); ++i) {
        if (!fBatches[i]) {
            continue;
        }
        if (fBatches[i]->renderTarget() != currentRT) {
            if (commandBuffer) {
                commandBuffer->end();
                // For now just use size of whole render target, but this should be updated to
                // only be the actual bounds of the various draws.
                SkIRect bounds = SkIRect::MakeWH(currentRT->width(), currentRT->height());
                commandBuffer->submit(bounds);
                commandBuffer.reset();
            }
            currentRT = fBatches[i]->renderTarget();
            if (currentRT) {
                static const GrGpuCommandBuffer::LoadAndStoreInfo kBasicLoadStoreInfo
                    { GrGpuCommandBuffer::LoadOp::kLoad,GrGpuCommandBuffer::StoreOp::kStore,
                      GrColor_ILLEGAL };
                commandBuffer.reset(fGpu->createCommandBuffer(currentRT,
                                                              kBasicLoadStoreInfo,   // Color
                                                              kBasicLoadStoreInfo)); // Stencil
            }
            flushState->setCommandBuffer(commandBuffer);
        }
        if (fDrawBatchBounds) {
            const SkRect& bounds = fBatches[i]->bounds();
            SkIRect ibounds;
            bounds.roundOut(&ibounds);
            // In multi-draw buffer all the batches use the same render target and we won't need to
            // get the batchs bounds.
            if (GrRenderTarget* rt = fBatches[i]->renderTarget()) {
                fGpu->drawDebugWireRect(rt, ibounds, 0xFF000000 | random.nextU());
            }
        }
        fBatches[i]->draw(flushState);
    }
    if (commandBuffer) {
        commandBuffer->end();
        // For now just use size of whole render target, but this should be updated to
        // only be the actual bounds of the various draws.
        SkIRect bounds = SkIRect::MakeWH(currentRT->width(), currentRT->height());
        commandBuffer->submit(bounds);
        flushState->setCommandBuffer(nullptr);
    }

    fGpu->finishDrawTarget();
}

void GrDrawTarget::reset() {
    fBatches.reset();
}

void GrDrawTarget::drawBatch(const GrPipelineBuilder& pipelineBuilder,
                             GrDrawContext* drawContext,
                             const GrClip& clip,
                             GrDrawBatch* batch) {
    // Setup clip
    GrAppliedClip appliedClip;
    if (!clip.apply(fContext, pipelineBuilder, drawContext, &batch->bounds(), &appliedClip)) {
        return;
    }

    // TODO: this is the only remaining usage of the AutoRestoreFragmentProcessorState - remove it
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    if (appliedClip.getClipCoverageFragmentProcessor()) {
        arfps.set(&pipelineBuilder);
        arfps.addCoverageFragmentProcessor(sk_ref_sp(appliedClip.getClipCoverageFragmentProcessor()));
    }

    GrPipeline::CreateArgs args;
    args.fPipelineBuilder = &pipelineBuilder;
    args.fDrawContext = drawContext;
    args.fCaps = this->caps();
    args.fScissor = &appliedClip.scissorState();
    args.fHasStencilClip = appliedClip.hasStencilClip();
    if (pipelineBuilder.hasUserStencilSettings() || appliedClip.hasStencilClip()) {
        if (!fResourceProvider->attachStencilAttachment(drawContext->accessRenderTarget())) {
            SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
            return;
        }
    }
    batch->getPipelineOptimizations(&args.fOpts);
    GrScissorState finalScissor;
    if (args.fOpts.fOverrides.fUsePLSDstRead || fClipBatchToBounds) {
        GrGLIRect viewport;
        viewport.fLeft = 0;
        viewport.fBottom = 0;
        viewport.fWidth = drawContext->width();
        viewport.fHeight = drawContext->height();
        SkIRect ibounds;
        ibounds.fLeft = SkTPin(SkScalarFloorToInt(batch->bounds().fLeft), viewport.fLeft,
                              viewport.fWidth);
        ibounds.fTop = SkTPin(SkScalarFloorToInt(batch->bounds().fTop), viewport.fBottom,
                             viewport.fHeight);
        ibounds.fRight = SkTPin(SkScalarCeilToInt(batch->bounds().fRight), viewport.fLeft,
                               viewport.fWidth);
        ibounds.fBottom = SkTPin(SkScalarCeilToInt(batch->bounds().fBottom), viewport.fBottom,
                                viewport.fHeight);
        if (appliedClip.scissorState().enabled()) {
            const SkIRect& scissorRect = appliedClip.scissorState().rect();
            if (!ibounds.intersect(scissorRect)) {
                return;
            }
        }
        finalScissor.set(ibounds);
        args.fScissor = &finalScissor;
    }
    args.fOpts.fColorPOI.completeCalculations(
        sk_sp_address_as_pointer_address(pipelineBuilder.fColorFragmentProcessors.begin()),
        pipelineBuilder.numColorFragmentProcessors());
    args.fOpts.fCoveragePOI.completeCalculations(
        sk_sp_address_as_pointer_address(pipelineBuilder.fCoverageFragmentProcessors.begin()),
        pipelineBuilder.numCoverageFragmentProcessors());
    if (!this->setupDstReadIfNecessary(pipelineBuilder, drawContext->accessRenderTarget(),
                                       clip, args.fOpts,
                                       &args.fDstTexture, batch->bounds())) {
        return;
    }

    if (!batch->installPipeline(args)) {
        return;
    }

#ifdef ENABLE_MDB
    SkASSERT(fRenderTarget);
    batch->pipeline()->addDependenciesTo(fRenderTarget);
#endif

    this->recordBatch(batch);
}

void GrDrawTarget::stencilPath(const GrPipelineBuilder& pipelineBuilder,
                               GrDrawContext* drawContext,
                               const GrClip& clip,
                               const SkMatrix& viewMatrix,
                               const GrPath* path,
                               GrPathRendering::FillType fill) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    // Setup clip
    GrAppliedClip appliedClip;
    if (!clip.apply(fContext, pipelineBuilder, drawContext, nullptr, &appliedClip)) {
        return;
    }
    // TODO: respect fClipBatchToBounds if we ever start computing bounds here.

    // Coverage AA does not make sense when rendering to the stencil buffer. The caller should never
    // attempt this in a situation that would require coverage AA.
    SkASSERT(!appliedClip.getClipCoverageFragmentProcessor());

    GrStencilAttachment* stencilAttachment = fResourceProvider->attachStencilAttachment(
                                                drawContext->accessRenderTarget());
    if (!stencilAttachment) {
        SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
        return;
    }

    GrBatch* batch = GrStencilPathBatch::Create(viewMatrix,
                                                pipelineBuilder.isHWAntialias(),
                                                fill,
                                                appliedClip.hasStencilClip(),
                                                stencilAttachment->bits(),
                                                appliedClip.scissorState(),
                                                drawContext->accessRenderTarget(),
                                                path);
    this->recordBatch(batch);
    batch->unref();
}

void GrDrawTarget::clear(const SkIRect* rect,
                         GrColor color,
                         bool canIgnoreRect,
                         GrDrawContext* drawContext) {
    SkIRect rtRect = SkIRect::MakeWH(drawContext->width(), drawContext->height());
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

    if (this->caps()->useDrawInsteadOfClear()) {
        // This works around a driver bug with clear by drawing a rect instead.
        // The driver will ignore a clear if it is the only thing rendered to a
        // target before the target is read.
        if (rect == &rtRect) {
            drawContext->discard();
        }

        // TODO: flip this into real draw!
        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

        SkRect scalarRect = SkRect::Make(*rect);
        SkAutoTUnref<GrDrawBatch> batch(
                GrRectBatchFactory::CreateNonAAFill(color, SkMatrix::I(), scalarRect,
                                                    nullptr, nullptr));
        this->drawBatch(pipelineBuilder, drawContext, GrNoClip(), batch);
    } else {
        GrBatch* batch = new GrClearBatch(*rect, color, drawContext->accessRenderTarget());
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

bool GrDrawTarget::copySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) {
    GrBatch* batch = GrCopySurfaceBatch::Create(dst, src, srcRect, dstPoint);
    if (!batch) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    this->recordBatch(batch);
    batch->unref();
    return true;
}

template <class Left, class Right> static bool intersect(const Left& a, const Right& b) {
    SkASSERT(a.fLeft <= a.fRight && a.fTop <= a.fBottom &&
             b.fLeft <= b.fRight && b.fTop <= b.fBottom);
    return a.fLeft < b.fRight && b.fLeft < a.fRight && a.fTop < b.fBottom && b.fTop < a.fBottom;
}

void GrDrawTarget::recordBatch(GrBatch* batch) {
    // A closed drawTarget should never receive new/more batches
    SkASSERT(!this->isClosed());

    // Check if there is a Batch Draw we can batch with by linearly searching back until we either
    // 1) check every draw
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADDBATCH(fAuditTrail, batch);
    GrBATCH_INFO("Re-Recording (%s, B%u)\n"
        "\tBounds LRTB (%f, %f, %f, %f)\n",
        batch->name(),
        batch->uniqueID(),
        batch->bounds().fLeft, batch->bounds().fRight,
        batch->bounds().fTop, batch->bounds().fBottom);
    GrBATCH_INFO(SkTabString(batch->dumpInfo(), 1).c_str());
    GrBATCH_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(fMaxBatchLookback, fBatches.count());
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
                GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(fAuditTrail, candidate, batch);
                return;
            }
            // Stop going backwards if we would cause a painter's order violation.
            // TODO: The bounds used here do not fully consider the clip. It may be advantageous
            // to clip each batch's bounds to the clip.
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
    GR_AUDIT_TRAIL_BATCHING_RESULT_NEW(fAuditTrail, batch);
    fBatches.push_back().reset(SkRef(batch));
}

void GrDrawTarget::forwardCombine() {
    for (int i = 0; i < fBatches.count() - 2; ++i) {
        GrBatch* batch = fBatches[i];
        int maxCandidateIdx = SkTMin(i + fMaxBatchLookahead, fBatches.count() - 1);
        int j = i + 1;
        while (true) {
            GrBatch* candidate = fBatches[j];
            // We cannot continue to search if the render target changes
            if (candidate->renderTargetUniqueID() != batch->renderTargetUniqueID()) {
                GrBATCH_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                             candidate->name(), candidate->uniqueID());
                break;
            }
            if (j == i +1) {
                // We assume batch would have combined with candidate when the candidate was added
                // via backwards combining in recordBatch.
                SkASSERT(!batch->combineIfPossible(candidate, *this->caps()));
            } else if (batch->combineIfPossible(candidate, *this->caps())) {
                GrBATCH_INFO("\t\tCombining with (%s, B%u)\n", candidate->name(),
                             candidate->uniqueID());
                GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(fAuditTrail, batch, candidate);
                fBatches[j].reset(SkRef(batch));
                fBatches[i].reset(nullptr);
                break;
            }
            // Stop going traversing if we would cause a painter's order violation.
            // TODO: The bounds used here do not fully consider the clip. It may be advantageous
            // to clip each batch's bounds to the clip.
            if (intersect(candidate->bounds(), batch->bounds())) {
                GrBATCH_INFO("\t\tIntersects with (%s, B%u)\n", candidate->name(),
                             candidate->uniqueID());
                break;
            }
            ++j;
            if (j > maxCandidateIdx) {
                GrBATCH_INFO("\t\tReached max lookahead or end of batch array %d\n", i);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawTarget::clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* rt) {
    GrBatch* batch = new GrClearStencilClipBatch(rect, insideClip, rt);
    this->recordBatch(batch);
    batch->unref();
}
