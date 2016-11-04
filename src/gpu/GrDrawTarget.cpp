/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawTarget.h"

#include "GrAppliedClip.h"
#include "GrAuditTrail.h"
#include "GrCaps.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrPath.h"
#include "GrPipeline.h"
#include "GrMemoryPool.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "gl/GrGLRenderTarget.h"

#include "SkStrokeRec.h"

#include "batches/GrClearBatch.h"
#include "batches/GrClearStencilClipBatch.h"
#include "batches/GrCopySurfaceBatch.h"
#include "batches/GrDiscardBatch.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrDrawPathBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrStencilPathBatch.h"

#include "instanced/InstancedRendering.h"

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most batching occurs within the first 10 comparisons.
static const int kDefaultMaxBatchLookback  = 10;
static const int kDefaultMaxBatchLookahead = 10;

GrDrawTarget::GrDrawTarget(GrRenderTarget* rt, GrGpu* gpu, GrResourceProvider* resourceProvider,
                           GrAuditTrail* auditTrail, const Options& options)
    : fLastFullClearBatch(nullptr)
    , fGpu(SkRef(gpu))
    , fResourceProvider(resourceProvider)
    , fAuditTrail(auditTrail)
    , fFlags(0)
    , fRenderTarget(rt) {
    // TODO: Stop extracting the context (currently needed by GrClip)
    fContext = fGpu->getContext();

    fClipBatchToBounds = options.fClipBatchToBounds;
    fDrawBatchBounds = options.fDrawBatchBounds;
    fMaxBatchLookback = (options.fMaxBatchLookback < 0) ? kDefaultMaxBatchLookback :
                                                          options.fMaxBatchLookback;
    fMaxBatchLookahead = (options.fMaxBatchLookahead < 0) ? kDefaultMaxBatchLookahead :
                                                           options.fMaxBatchLookahead;

    if (GrCaps::InstancedSupport::kNone != this->caps()->instancedSupport()) {
        fInstancedRendering.reset(fGpu->createInstancedRendering());
    }

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
    SkDebugf("node: %d -> RT: %d\n", fDebugID, fRenderTarget ? fRenderTarget->uniqueID() : -1);
    SkDebugf("relies On (%d): ", fDependencies.count());
    for (int i = 0; i < fDependencies.count(); ++i) {
        SkDebugf("%d, ", fDependencies[i]->fDebugID);
    }
    SkDebugf("\n");
    SkDebugf("batches (%d):\n", fRecordedBatches.count());
    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fRecordedBatches[i].fBatch) {
            SkDebugf("%d: <combined forward>\n", i);
        } else {
            SkDebugf("%d: %s\n", i, fRecordedBatches[i].fBatch->name());
            SkString str = fRecordedBatches[i].fBatch->dumpInfo();
            SkDebugf("%s\n", str.c_str());
            const SkRect& clippedBounds = fRecordedBatches[i].fClippedBounds;
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                     clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                     clippedBounds.fBottom);
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
    if (!fGpu->initDescForDstCopy(rt, &desc)) {
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
    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        if (fRecordedBatches[i].fBatch) {
            fRecordedBatches[i].fBatch->prepare(flushState);
        }
    }

    if (fInstancedRendering) {
        fInstancedRendering->beginFlush(flushState->resourceProvider());
    }
}

bool GrDrawTarget::drawBatches(GrBatchFlushState* flushState) {
    if (0 == fRecordedBatches.count()) {
        return false;
    }
    // Draw all the generated geometry.
    SkRandom random;
    GrRenderTarget* currentRT = nullptr;
    SkAutoTDelete<GrGpuCommandBuffer> commandBuffer;
    SkRect bounds = SkRect::MakeEmpty();
    for (int i = 0; i < fRecordedBatches.count(); ++i) {
        if (!fRecordedBatches[i].fBatch) {
            continue;
        }
        if (fRecordedBatches[i].fBatch->renderTarget() != currentRT) {
            if (commandBuffer) {
                commandBuffer->end();
                if (bounds.intersect(0, 0,
                                     SkIntToScalar(currentRT->width()),
                                     SkIntToScalar(currentRT->height()))) {
                    SkIRect iBounds;
                    bounds.roundOut(&iBounds);
                    commandBuffer->submit(iBounds);
                }
                commandBuffer.reset();
            }
            bounds.setEmpty();
            currentRT = fRecordedBatches[i].fBatch->renderTarget();
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
        if (commandBuffer) {
            bounds.join(fRecordedBatches[i].fClippedBounds);
        }
        if (fDrawBatchBounds) {
            const SkRect& bounds = fRecordedBatches[i].fClippedBounds;
            SkIRect ibounds;
            bounds.roundOut(&ibounds);
            // In multi-draw buffer all the batches use the same render target and we won't need to
            // get the batchs bounds.
            if (GrRenderTarget* rt = fRecordedBatches[i].fBatch->renderTarget()) {
                fGpu->drawDebugWireRect(rt, ibounds, 0xFF000000 | random.nextU());
            }
        }
        fRecordedBatches[i].fBatch->draw(flushState);
    }
    if (commandBuffer) {
        commandBuffer->end();
        if (bounds.intersect(0, 0,
                             SkIntToScalar(currentRT->width()),
                             SkIntToScalar(currentRT->height()))) {
            SkIRect iBounds;
            bounds.roundOut(&iBounds);
            commandBuffer->submit(iBounds);
        }
        flushState->setCommandBuffer(nullptr);
    }

    fGpu->finishDrawTarget();
    return true;
}

void GrDrawTarget::reset() {
    fLastFullClearBatch = nullptr;
    fRecordedBatches.reset();
    if (fInstancedRendering) {
        fInstancedRendering->endFlush();
    }
}

static void batch_bounds(SkRect* bounds, const GrBatch* batch) {
    *bounds = batch->bounds();
    if (batch->hasZeroArea()) {
        if (batch->hasAABloat()) {
            bounds->outset(0.5f, 0.5f);
        } else {
            // We don't know which way the particular GPU will snap lines or points at integer
            // coords. So we ensure that the bounds is large enough for either snap.
            SkRect before = *bounds;
            bounds->roundOut(bounds);
            if (bounds->fLeft == before.fLeft) {
                bounds->fLeft -= 1;
            }
            if (bounds->fTop == before.fTop) {
                bounds->fTop -= 1;
            }
            if (bounds->fRight == before.fRight) {
                bounds->fRight += 1;
            }
            if (bounds->fBottom == before.fBottom) {
                bounds->fBottom += 1;
            }
        }
    }
}

void GrDrawTarget::drawBatch(const GrPipelineBuilder& pipelineBuilder,
                             GrDrawContext* drawContext,
                             const GrClip& clip,
                             GrDrawBatch* batch) {
    // Setup clip
    SkRect bounds;
    batch_bounds(&bounds, batch);
    GrAppliedClip appliedClip(bounds);
    if (!clip.apply(fContext, drawContext, pipelineBuilder.isHWAntialias(),
                    pipelineBuilder.hasUserStencilSettings(), &appliedClip)) {
        return;
    }

    // TODO: this is the only remaining usage of the AutoRestoreFragmentProcessorState - remove it
    GrPipelineBuilder::AutoRestoreFragmentProcessorState arfps;
    if (appliedClip.clipCoverageFragmentProcessor()) {
        arfps.set(&pipelineBuilder);
        arfps.addCoverageFragmentProcessor(sk_ref_sp(appliedClip.clipCoverageFragmentProcessor()));
    }

    if (pipelineBuilder.hasUserStencilSettings() || appliedClip.hasStencilClip()) {
        if (!fResourceProvider->attachStencilAttachment(drawContext->accessRenderTarget())) {
            SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
            return;
        }
    }

    GrPipeline::CreateArgs args;
    args.fPipelineBuilder = &pipelineBuilder;
    args.fDrawContext = drawContext;
    args.fCaps = this->caps();
    batch->getPipelineOptimizations(&args.fOpts);
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
        if (!appliedClip.addScissor(ibounds)) {
            return;
        }
    }
    args.fOpts.fColorPOI.completeCalculations(
        sk_sp_address_as_pointer_address(pipelineBuilder.fColorFragmentProcessors.begin()),
        pipelineBuilder.numColorFragmentProcessors());
    args.fOpts.fCoveragePOI.completeCalculations(
        sk_sp_address_as_pointer_address(pipelineBuilder.fCoverageFragmentProcessors.begin()),
        pipelineBuilder.numCoverageFragmentProcessors());
    args.fScissor = &appliedClip.scissorState();
    args.fWindowRectsState = &appliedClip.windowRectsState();
    args.fHasStencilClip = appliedClip.hasStencilClip();
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
    this->recordBatch(batch, appliedClip.clippedDrawBounds());
}

void GrDrawTarget::stencilPath(GrDrawContext* drawContext,
                               const GrClip& clip,
                               bool useHWAA,
                               const SkMatrix& viewMatrix,
                               const GrPath* path) {
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    // FIXME: Use path bounds instead of this WAR once
    // https://bugs.chromium.org/p/skia/issues/detail?id=5640 is resolved.
    SkRect bounds = SkRect::MakeIWH(drawContext->width(), drawContext->height());

    // Setup clip
    GrAppliedClip appliedClip(bounds);
    if (!clip.apply(fContext, drawContext, useHWAA, true, &appliedClip)) {
        return;
    }
    // TODO: respect fClipBatchToBounds if we ever start computing bounds here.

    // Coverage AA does not make sense when rendering to the stencil buffer. The caller should never
    // attempt this in a situation that would require coverage AA.
    SkASSERT(!appliedClip.clipCoverageFragmentProcessor());

    GrStencilAttachment* stencilAttachment = fResourceProvider->attachStencilAttachment(
                                                drawContext->accessRenderTarget());
    if (!stencilAttachment) {
        SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
        return;
    }

    GrBatch* batch = GrStencilPathBatch::Create(viewMatrix,
                                                useHWAA,
                                                path->getFillType(),
                                                appliedClip.hasStencilClip(),
                                                stencilAttachment->bits(),
                                                appliedClip.scissorState(),
                                                drawContext->accessRenderTarget(),
                                                path);
    this->recordBatch(batch, appliedClip.clippedDrawBounds());
    batch->unref();
}

void GrDrawTarget::addBatch(sk_sp<GrBatch> batch) {
    this->recordBatch(batch.get(), batch->bounds());
}

void GrDrawTarget::fullClear(GrRenderTarget* renderTarget, GrColor color) {
    // Currently this just inserts or updates the last clear batch. However, once in MDB this can
    // remove all the previously recorded batches and change the load op to clear with supplied
    // color.
    if (fLastFullClearBatch &&
        fLastFullClearBatch->renderTargetUniqueID() == renderTarget->uniqueID()) {
        // As currently implemented, fLastFullClearBatch should be the last batch because we would
        // have cleared it when another batch was recorded.
        SkASSERT(fRecordedBatches.back().fBatch.get() == fLastFullClearBatch);
        fLastFullClearBatch->setColor(color);
        return;
    }
    sk_sp<GrClearBatch> batch(GrClearBatch::Make(GrFixedClip::Disabled(), color, renderTarget));
    if (batch.get() == this->recordBatch(batch.get(), batch->bounds())) {
        fLastFullClearBatch = batch.get();
    }
}

void GrDrawTarget::discard(GrRenderTarget* renderTarget) {
    // Currently this just inserts a discard batch. However, once in MDB this can remove all the
    // previously recorded batches and change the load op to discard.
    if (this->caps()->discardRenderTargetSupport()) {
        GrBatch* batch = new GrDiscardBatch(renderTarget);
        this->recordBatch(batch, batch->bounds());
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

    this->recordBatch(batch, batch->bounds());
    batch->unref();
    return true;
}

static inline bool can_reorder(const SkRect& a, const SkRect& b) {
    return a.fRight <= b.fLeft || a.fBottom <= b.fTop ||
           b.fRight <= a.fLeft || b.fBottom <= a.fTop;
}

static void join(SkRect* out, const SkRect& a, const SkRect& b) {
    SkASSERT(a.fLeft <= a.fRight && a.fTop <= a.fBottom);
    SkASSERT(b.fLeft <= b.fRight && b.fTop <= b.fBottom);
    out->fLeft   = SkTMin(a.fLeft,   b.fLeft);
    out->fTop    = SkTMin(a.fTop,    b.fTop);
    out->fRight  = SkTMax(a.fRight,  b.fRight);
    out->fBottom = SkTMax(a.fBottom, b.fBottom);
}

GrBatch* GrDrawTarget::recordBatch(GrBatch* batch, const SkRect& clippedBounds) {
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
    GrBATCH_INFO("\tClipped Bounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                 clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                 clippedBounds.fBottom);
    GrBATCH_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(fMaxBatchLookback, fRecordedBatches.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            GrBatch* candidate = fRecordedBatches.fromBack(i).fBatch.get();
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
                join(&fRecordedBatches.fromBack(i).fClippedBounds,
                     fRecordedBatches.fromBack(i).fClippedBounds, clippedBounds);
                return candidate;
            }
            // Stop going backwards if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedBatches.fromBack(i).fClippedBounds;
            if (!can_reorder(candidateBounds, clippedBounds)) {
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
    fRecordedBatches.emplace_back(RecordedBatch{sk_ref_sp(batch), clippedBounds});
    fLastFullClearBatch = nullptr;
    return batch;
}

void GrDrawTarget::forwardCombine() {
    if (fMaxBatchLookahead <= 0) {
        return;
    }
    for (int i = 0; i < fRecordedBatches.count() - 2; ++i) {
        GrBatch* batch = fRecordedBatches[i].fBatch.get();
        const SkRect& batchBounds = fRecordedBatches[i].fClippedBounds;
        int maxCandidateIdx = SkTMin(i + fMaxBatchLookahead, fRecordedBatches.count() - 1);
        int j = i + 1;
        while (true) {
            GrBatch* candidate = fRecordedBatches[j].fBatch.get();
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
                fRecordedBatches[j].fBatch = std::move(fRecordedBatches[i].fBatch);
                join(&fRecordedBatches[j].fClippedBounds, fRecordedBatches[j].fClippedBounds,
                     batchBounds);
                break;
            }
            // Stop going traversing if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedBatches[j].fClippedBounds;
            if (!can_reorder(candidateBounds, batchBounds)) {
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

void GrDrawTarget::clearStencilClip(const GrFixedClip& clip,
                                    bool insideStencilMask,
                                    GrRenderTarget* rt) {
    GrBatch* batch = new GrClearStencilClipBatch(clip, insideStencilMask, rt);
    this->recordBatch(batch, batch->bounds());
    batch->unref();
}
