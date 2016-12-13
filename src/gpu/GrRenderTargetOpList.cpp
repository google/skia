/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetOpList.h"

#include "GrAppliedClip.h"
#include "GrAuditTrail.h"
#include "GrCaps.h"
#include "GrRenderTargetContext.h"
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
#include "batches/GrDrawOp.h"
#include "batches/GrDrawPathBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "batches/GrStencilPathBatch.h"

#include "instanced/InstancedRendering.h"

using gr_instanced::InstancedRendering;

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most batching occurs within the first 10 comparisons.
static const int kDefaultMaxBatchLookback  = 10;
static const int kDefaultMaxBatchLookahead = 10;

GrRenderTargetOpList::GrRenderTargetOpList(GrRenderTargetProxy* rtp, GrGpu* gpu,
                                           GrResourceProvider* resourceProvider,
                                           GrAuditTrail* auditTrail, const Options& options)
    : INHERITED(rtp, auditTrail)
    , fLastFullClearOp(nullptr)
    , fGpu(SkRef(gpu))
    , fResourceProvider(resourceProvider)
    , fLastClipStackGenID(SK_InvalidUniqueID) {
    // TODO: Stop extracting the context (currently needed by GrClip)
    fContext = fGpu->getContext();

    fClipOpToBounds = options.fClipBatchToBounds;
    fMaxOpLookback = (options.fMaxBatchLookback < 0) ? kDefaultMaxBatchLookback :
                                                          options.fMaxBatchLookback;
    fMaxOpLookahead = (options.fMaxBatchLookahead < 0) ? kDefaultMaxBatchLookahead :
                                                           options.fMaxBatchLookahead;

    if (GrCaps::InstancedSupport::kNone != this->caps()->instancedSupport()) {
        fInstancedRendering.reset(fGpu->createInstancedRendering());
    }
}

GrRenderTargetOpList::~GrRenderTargetOpList() {
    fGpu->unref();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrRenderTargetOpList::dump() const {
    INHERITED::dump();

    SkDebugf("ops (%d):\n", fRecordedOps.count());
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fRecordedOps[i].fOp) {
            SkDebugf("%d: <combined forward>\n", i);
        } else {
            SkDebugf("%d: %s\n", i, fRecordedOps[i].fOp->name());
            SkString str = fRecordedOps[i].fOp->dumpInfo();
            SkDebugf("%s\n", str.c_str());
            const SkRect& clippedBounds = fRecordedOps[i].fClippedBounds;
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                     clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
                     clippedBounds.fBottom);
        }
    }
}
#endif

void GrRenderTargetOpList::setupDstTexture(GrRenderTarget* rt,
                                           const GrClip& clip,
                                           const SkRect& batchBounds,
                                           GrXferProcessor::DstTexture* dstTexture) {
    SkRect bounds = batchBounds;
    bounds.outset(0.5f, 0.5f);

    if (this->caps()->textureBarrierSupport()) {
        if (GrTexture* rtTex = rt->asTexture()) {
            // The render target is a texture, so we can read from it directly in the shader. The XP
            // will be responsible to detect this situation and request a texture barrier.
            dstTexture->setTexture(sk_ref_sp(rtTex));
            dstTexture->setOffset(0, 0);
            return;
        }
    }

    SkIRect copyRect;
    clip.getConservativeBounds(rt->width(), rt->height(), &copyRect);

    SkIRect drawIBounds;
    bounds.roundOut(&drawIBounds);
    if (!copyRect.intersect(drawIBounds)) {
#ifdef SK_DEBUG
        GrCapsDebugf(this->caps(), "Missed an early reject. "
                                   "Bailing on draw from setupDstTexture.\n");
#endif
        return;
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
    sk_sp<GrTexture> copy(fResourceProvider->createApproxTexture(desc, kFlags));

    if (!copy) {
        SkDebugf("Failed to create temporary copy of destination texture.\n");
        return;
    }
    SkIPoint dstPoint = {0, 0};
    this->copySurface(copy.get(), rt, copyRect, dstPoint);
    dstTexture->setTexture(std::move(copy));
    dstTexture->setOffset(copyRect.fLeft, copyRect.fTop);
}

void GrRenderTargetOpList::prepareOps(GrOpFlushState* flushState) {
    // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
    // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
    // but need to be flushed anyway. Closing such GrOpLists here will mean new
    // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
    this->makeClosed();

    // Loop over the ops that haven't yet been prepared.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i].fOp) {
            fRecordedOps[i].fOp->prepare(flushState);
        }
    }

    if (fInstancedRendering) {
        fInstancedRendering->beginFlush(flushState->resourceProvider());
    }
}

// TODO: this is where GrOp::renderTarget is used (which is fine since it
// is at flush time). However, we need to store the RenderTargetProxy in the
// Ops and instantiate them here.
bool GrRenderTargetOpList::executeOps(GrOpFlushState* flushState) {
    if (0 == fRecordedOps.count()) {
        return false;
    }
    // Draw all the generated geometry.
    SkRandom random;
    GrGpuResource::UniqueID currentRTID = GrGpuResource::UniqueID::InvalidID();
    std::unique_ptr<GrGpuCommandBuffer> commandBuffer;
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i].fOp) {
            continue;
        }
        if (fRecordedOps[i].fOp->renderTargetUniqueID() != currentRTID) {
            if (commandBuffer) {
                commandBuffer->end();
                commandBuffer->submit();
                commandBuffer.reset();
            }
            currentRTID = fRecordedOps[i].fOp->renderTargetUniqueID();
            if (!currentRTID.isInvalid()) {
                static const GrGpuCommandBuffer::LoadAndStoreInfo kBasicLoadStoreInfo
                    { GrGpuCommandBuffer::LoadOp::kLoad,GrGpuCommandBuffer::StoreOp::kStore,
                      GrColor_ILLEGAL };
                commandBuffer.reset(fGpu->createCommandBuffer(kBasicLoadStoreInfo,   // Color
                                                              kBasicLoadStoreInfo)); // Stencil
            }
            flushState->setCommandBuffer(commandBuffer.get());
        }
        fRecordedOps[i].fOp->draw(flushState, fRecordedOps[i].fClippedBounds);
    }
    if (commandBuffer) {
        commandBuffer->end();
        commandBuffer->submit();
        flushState->setCommandBuffer(nullptr);
    }

    fGpu->finishOpList();
    return true;
}

void GrRenderTargetOpList::reset() {
    fLastFullClearOp = nullptr;
    fRecordedOps.reset();
    if (fInstancedRendering) {
        fInstancedRendering->endFlush();
    }
}

void GrRenderTargetOpList::abandonGpuResources() {
    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->instancedRendering();
        ir->resetGpuResources(InstancedRendering::ResetType::kAbandon);
    }
}

void GrRenderTargetOpList::freeGpuResources() {
    if (GrCaps::InstancedSupport::kNone != fContext->caps()->instancedSupport()) {
        InstancedRendering* ir = this->instancedRendering();
        ir->resetGpuResources(InstancedRendering::ResetType::kDestroy);
    }
}

static void batch_bounds(SkRect* bounds, const GrOp* batch) {
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

void GrRenderTargetOpList::addDrawOp(const GrPipelineBuilder& pipelineBuilder,
                                     GrRenderTargetContext* renderTargetContext,
                                     const GrClip& clip,
                                     sk_sp<GrDrawOp> op) {
    // Setup clip
    SkRect bounds;
    batch_bounds(&bounds, op.get());
    GrAppliedClip appliedClip(bounds);
    if (!clip.apply(fContext, renderTargetContext, pipelineBuilder.isHWAntialias(),
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
        if (!renderTargetContext->accessRenderTarget()) {
            return;
        }

        if (!fResourceProvider->attachStencilAttachment(
                renderTargetContext->accessRenderTarget())) {
            SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
            return;
        }
    }

    GrPipeline::CreateArgs args;
    args.fPipelineBuilder = &pipelineBuilder;
    args.fRenderTargetContext = renderTargetContext;
    args.fCaps = this->caps();
    op->getPipelineOptimizations(&args.fOpts);
    if (args.fOpts.fOverrides.fUsePLSDstRead || fClipOpToBounds) {
        GrGLIRect viewport;
        viewport.fLeft = 0;
        viewport.fBottom = 0;
        viewport.fWidth = renderTargetContext->width();
        viewport.fHeight = renderTargetContext->height();
        SkIRect ibounds;
        ibounds.fLeft = SkTPin(SkScalarFloorToInt(op->bounds().fLeft), viewport.fLeft,
                              viewport.fWidth);
        ibounds.fTop = SkTPin(SkScalarFloorToInt(op->bounds().fTop), viewport.fBottom,
                             viewport.fHeight);
        ibounds.fRight = SkTPin(SkScalarCeilToInt(op->bounds().fRight), viewport.fLeft,
                               viewport.fWidth);
        ibounds.fBottom = SkTPin(SkScalarCeilToInt(op->bounds().fBottom), viewport.fBottom,
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
    if (!renderTargetContext->accessRenderTarget()) {
        return;
    }

    if (pipelineBuilder.willXPNeedDstTexture(*this->caps(), args.fOpts)) {
        this->setupDstTexture(renderTargetContext->accessRenderTarget(), clip, op->bounds(),
                              &args.fDstTexture);
        if (!args.fDstTexture.texture()) {
            return;
        }
    }

    if (!op->installPipeline(args)) {
        return;
    }

#ifdef ENABLE_MDB
    SkASSERT(fSurface);
    op->pipeline()->addDependenciesTo(fSurface);
#endif
    this->recordOp(std::move(op), appliedClip.clippedDrawBounds());
}

void GrRenderTargetOpList::stencilPath(GrRenderTargetContext* renderTargetContext,
                                       const GrClip& clip,
                                       GrAAType aaType,
                                       const SkMatrix& viewMatrix,
                                       const GrPath* path) {
    bool useHWAA = GrAATypeIsHW(aaType);
    // TODO: extract portions of checkDraw that are relevant to path stenciling.
    SkASSERT(path);
    SkASSERT(this->caps()->shaderCaps()->pathRenderingSupport());

    // FIXME: Use path bounds instead of this WAR once
    // https://bugs.chromium.org/p/skia/issues/detail?id=5640 is resolved.
    SkRect bounds = SkRect::MakeIWH(renderTargetContext->width(), renderTargetContext->height());

    // Setup clip
    GrAppliedClip appliedClip(bounds);
    if (!clip.apply(fContext, renderTargetContext, useHWAA, true, &appliedClip)) {
        return;
    }
    // TODO: respect fClipOpToBounds if we ever start computing bounds here.

    // Coverage AA does not make sense when rendering to the stencil buffer. The caller should never
    // attempt this in a situation that would require coverage AA.
    SkASSERT(!appliedClip.clipCoverageFragmentProcessor());

    if (!renderTargetContext->accessRenderTarget()) {
        return;
    }
    GrStencilAttachment* stencilAttachment = fResourceProvider->attachStencilAttachment(
                                                renderTargetContext->accessRenderTarget());
    if (!stencilAttachment) {
        SkDebugf("ERROR creating stencil attachment. Draw skipped.\n");
        return;
    }

    sk_sp<GrOp> op = GrStencilPathBatch::Make(viewMatrix,
                                              useHWAA,
                                              path->getFillType(),
                                              appliedClip.hasStencilClip(),
                                              stencilAttachment->bits(),
                                              appliedClip.scissorState(),
                                              renderTargetContext->accessRenderTarget(),
                                              path);
    this->recordOp(std::move(op), appliedClip.clippedDrawBounds());
}

void GrRenderTargetOpList::fullClear(GrRenderTarget* renderTarget, GrColor color) {
    // Currently this just inserts or updates the last clear op. However, once in MDB this can
    // remove all the previously recorded ops and change the load op to clear with supplied
    // color.
    // TODO: this needs to be updated to use GrSurfaceProxy::UniqueID
    if (fLastFullClearOp &&
        fLastFullClearOp->renderTargetUniqueID() == renderTarget->uniqueID()) {
        // As currently implemented, fLastFullClearOp should be the last op because we would
        // have cleared it when another op was recorded.
        SkASSERT(fRecordedOps.back().fOp.get() == fLastFullClearOp);
        fLastFullClearOp->setColor(color);
        return;
    }
    sk_sp<GrClearBatch> op(GrClearBatch::Make(GrFixedClip::Disabled(), color, renderTarget));
    if (GrOp* clearOp = this->recordOp(std::move(op))) {
        // This is either the clear op we just created or another one that it combined with.
        fLastFullClearOp = static_cast<GrClearBatch*>(clearOp);
    }
}

void GrRenderTargetOpList::discard(GrRenderTarget* renderTarget) {
    // Currently this just inserts a discard op. However, once in MDB this can remove all the
    // previously recorded ops and change the load op to discard.
    if (this->caps()->discardRenderTargetSupport()) {
        this->recordOp(GrDiscardBatch::Make(renderTarget));
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetOpList::copySurface(GrSurface* dst,
                                       GrSurface* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    sk_sp<GrOp> op = GrCopySurfaceBatch::Make(dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    this->recordOp(std::move(op));
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

GrOp* GrRenderTargetOpList::recordOp(sk_sp<GrOp> op, const SkRect& clippedBounds) {
    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADDBATCH(fAuditTrail, op.get());
    GrOP_INFO("Re-Recording (%s, B%u)\n"
              "\tBounds LRTB (%f, %f, %f, %f)\n",
               op->name(),
               op->uniqueID(),
               op->bounds().fLeft, op->bounds().fRight,
               op->bounds().fTop, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GrOP_INFO("\tClipped Bounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
              clippedBounds.fLeft, clippedBounds.fTop, clippedBounds.fRight,
              clippedBounds.fBottom);
    GrOP_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(fMaxOpLookback, fRecordedOps.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            GrOp* candidate = fRecordedOps.fromBack(i).fOp.get();
            // We cannot continue to search backwards if the render target changes
            if (candidate->renderTargetUniqueID() != op->renderTargetUniqueID()) {
                GrOP_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                          candidate->name(), candidate->uniqueID());
                break;
            }
            if (candidate->combineIfPossible(op.get(), *this->caps())) {
                GrOP_INFO("\t\tCombining with (%s, B%u)\n", candidate->name(),
                          candidate->uniqueID());
                GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(fAuditTrail, candidate, op.get());
                join(&fRecordedOps.fromBack(i).fClippedBounds,
                     fRecordedOps.fromBack(i).fClippedBounds, clippedBounds);
                return candidate;
            }
            // Stop going backwards if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedOps.fromBack(i).fClippedBounds;
            if (!can_reorder(candidateBounds, clippedBounds)) {
                GrOP_INFO("\t\tIntersects with (%s, B%u)\n", candidate->name(),
                          candidate->uniqueID());
                break;
            }
            ++i;
            if (i == maxCandidates) {
                GrOP_INFO("\t\tReached max lookback or beginning of op array %d\n", i);
                break;
            }
        }
    } else {
        GrOP_INFO("\t\tFirstOp\n");
    }
    GR_AUDIT_TRAIL_BATCHING_RESULT_NEW(fAuditTrail, op);
    fRecordedOps.emplace_back(RecordedOp{std::move(op), clippedBounds});
    fLastFullClearOp = nullptr;
    return fRecordedOps.back().fOp.get();
}

void GrRenderTargetOpList::forwardCombine() {
    if (fMaxOpLookahead <= 0) {
        return;
    }
    for (int i = 0; i < fRecordedOps.count() - 2; ++i) {
        GrOp* op = fRecordedOps[i].fOp.get();
        const SkRect& opBounds = fRecordedOps[i].fClippedBounds;
        int maxCandidateIdx = SkTMin(i + fMaxOpLookahead, fRecordedOps.count() - 1);
        int j = i + 1;
        while (true) {
            GrOp* candidate = fRecordedOps[j].fOp.get();
            // We cannot continue to search if the render target changes
            if (candidate->renderTargetUniqueID() != op->renderTargetUniqueID()) {
                GrOP_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n",
                          candidate->name(), candidate->uniqueID());
                break;
            }
            if (j == i +1) {
                // We assume op would have combined with candidate when the candidate was added
                // via backwards combining in recordOp.
                SkASSERT(!op->combineIfPossible(candidate, *this->caps()));
            } else if (op->combineIfPossible(candidate, *this->caps())) {
                GrOP_INFO("\t\tCombining with (%s, B%u)\n", candidate->name(),
                          candidate->uniqueID());
                GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(fAuditTrail, op, candidate);
                fRecordedOps[j].fOp = std::move(fRecordedOps[i].fOp);
                join(&fRecordedOps[j].fClippedBounds, fRecordedOps[j].fClippedBounds, opBounds);
                break;
            }
            // Stop going traversing if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedOps[j].fClippedBounds;
            if (!can_reorder(candidateBounds, opBounds)) {
                GrOP_INFO("\t\tIntersects with (%s, B%u)\n", candidate->name(),
                          candidate->uniqueID());
                break;
            }
            ++j;
            if (j > maxCandidateIdx) {
                GrOP_INFO("\t\tReached max lookahead or end of batch array %d\n", i);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetOpList::clearStencilClip(const GrFixedClip& clip,
                                            bool insideStencilMask,
                                            GrRenderTarget* rt) {
    this->recordOp(GrClearStencilClipBatch::Make(clip, insideStencilMask, rt));
}
