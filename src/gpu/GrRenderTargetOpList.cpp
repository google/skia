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

#include "ops/GrClearOp.h"
#include "ops/GrClearStencilClipOp.h"
#include "ops/GrCopySurfaceOp.h"
#include "ops/GrDiscardOp.h"
#include "ops/GrDrawOp.h"
#include "ops/GrDrawPathOp.h"
#include "ops/GrRectOpFactory.h"
#include "ops/GrStencilPathOp.h"

#include "instanced/InstancedRendering.h"

using gr_instanced::InstancedRendering;

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kDefaultMaxOpLookback = 10;
static const int kDefaultMaxOpLookahead = 10;

GrRenderTargetOpList::GrRenderTargetOpList(GrRenderTargetProxy* rtp, GrGpu* gpu,
                                           GrResourceProvider* resourceProvider,
                                           GrAuditTrail* auditTrail, const Options& options)
    : INHERITED(rtp, auditTrail)
    , fGpu(SkRef(gpu))
    , fResourceProvider(resourceProvider)
    , fLastClipStackGenID(SK_InvalidUniqueID) {
    // TODO: Stop extracting the context (currently needed by GrClip)
    fContext = fGpu->getContext();

    fClipOpToBounds = options.fClipDrawOpsToBounds;
    fMaxOpLookback = (options.fMaxOpCombineLookback < 0) ? kDefaultMaxOpLookback
                                                         : options.fMaxOpCombineLookback;
    fMaxOpLookahead = (options.fMaxOpCombineLookahead < 0) ? kDefaultMaxOpLookahead
                                                           : options.fMaxOpCombineLookahead;

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
                                           const SkRect& opBounds,
                                           GrXferProcessor::DstTexture* dstTexture) {
    SkRect bounds = opBounds;
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
    const GrRenderTarget* currentRenderTarget = nullptr;
    std::unique_ptr<GrGpuCommandBuffer> commandBuffer;
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i].fOp) {
            continue;
        }
        if (fRecordedOps[i].fRenderTarget.get() != currentRenderTarget) {
            if (commandBuffer) {
                commandBuffer->end();
                commandBuffer->submit();
                commandBuffer.reset();
            }
            currentRenderTarget = fRecordedOps[i].fRenderTarget.get();
            if (currentRenderTarget) {
                static const GrGpuCommandBuffer::LoadAndStoreInfo kBasicLoadStoreInfo
                    { GrGpuCommandBuffer::LoadOp::kLoad,GrGpuCommandBuffer::StoreOp::kStore,
                      GrColor_ILLEGAL };
                commandBuffer.reset(fGpu->createCommandBuffer(kBasicLoadStoreInfo,   // Color
                                                              kBasicLoadStoreInfo)); // Stencil
            }
            flushState->setCommandBuffer(commandBuffer.get());
        }
        fRecordedOps[i].fOp->execute(flushState, fRecordedOps[i].fClippedBounds);
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
    fLastFullClearRenderTargetID.makeInvalid();
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

static void op_bounds(SkRect* bounds, const GrOp* op) {
    *bounds = op->bounds();
    if (op->hasZeroArea()) {
        if (op->hasAABloat()) {
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
                                     std::unique_ptr<GrDrawOp> op) {
    // Setup clip
    SkRect bounds;
    op_bounds(&bounds, op.get());
    GrAppliedClip appliedClip(bounds);
    if (!clip.apply(fContext, renderTargetContext, pipelineBuilder.isHWAntialias(),
                    pipelineBuilder.hasUserStencilSettings(), &appliedClip)) {
        return;
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

    GrProcessorSet::FragmentProcessorAnalysis analysis;
    op->analyzeProcessors(&analysis, pipelineBuilder.processors(), appliedClip, *this->caps());

    GrPipeline::InitArgs args;
    pipelineBuilder.getPipelineInitArgs(&args);
    args.fAppliedClip = &appliedClip;
    // This forces instantiation of the render target. Pipeline creation is moving to flush time
    // by which point instantiation must have occurred anyway.
    args.fRenderTarget = renderTargetContext->accessRenderTarget();
    if (!args.fRenderTarget) {
        return;
    }
    args.fCaps = this->caps();
    args.fAnalysis = &analysis;
    if (analysis.usesPLSDstRead() || fClipOpToBounds) {
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

    if (!renderTargetContext->accessRenderTarget()) {
        return;
    }

    if (pipelineBuilder.willXPNeedDstTexture(*this->caps(), analysis)) {
        this->setupDstTexture(renderTargetContext->accessRenderTarget(), clip, op->bounds(),
                              &args.fDstTexture);
        if (!args.fDstTexture.texture()) {
            return;
        }
    }
    op->initPipeline(args);

#ifdef ENABLE_MDB
    SkASSERT(fSurface);
    op->pipeline()->addDependenciesTo(fSurface);
#endif
    this->recordOp(std::move(op), renderTargetContext, appliedClip.clippedDrawBounds());
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

    std::unique_ptr<GrOp> op = GrStencilPathOp::Make(viewMatrix,
                                                     useHWAA,
                                                     path->getFillType(),
                                                     appliedClip.hasStencilClip(),
                                                     stencilAttachment->bits(),
                                                     appliedClip.scissorState(),
                                                     renderTargetContext->accessRenderTarget(),
                                                     path);
    this->recordOp(std::move(op), renderTargetContext, appliedClip.clippedDrawBounds());
}

void GrRenderTargetOpList::fullClear(GrRenderTargetContext* renderTargetContext, GrColor color) {
    GrRenderTarget* renderTarget = renderTargetContext->accessRenderTarget();
    // Currently this just inserts or updates the last clear op. However, once in MDB this can
    // remove all the previously recorded ops and change the load op to clear with supplied
    // color.
    // TODO: this needs to be updated to use GrSurfaceProxy::UniqueID
    if (fLastFullClearRenderTargetID == renderTarget->uniqueID()) {
        // As currently implemented, fLastFullClearOp should be the last op because we would
        // have cleared it when another op was recorded.
        SkASSERT(fRecordedOps.back().fOp.get() == fLastFullClearOp);
        fLastFullClearOp->setColor(color);
        return;
    }
    std::unique_ptr<GrClearOp> op(GrClearOp::Make(GrFixedClip::Disabled(), color, renderTarget));
    if (GrOp* clearOp = this->recordOp(std::move(op), renderTargetContext)) {
        // This is either the clear op we just created or another one that it combined with.
        fLastFullClearOp = static_cast<GrClearOp*>(clearOp);
        fLastFullClearRenderTargetID = renderTarget->uniqueID();
    }
}

void GrRenderTargetOpList::discard(GrRenderTargetContext* renderTargetContext) {
    // Currently this just inserts a discard op. However, once in MDB this can remove all the
    // previously recorded ops and change the load op to discard.
    if (this->caps()->discardRenderTargetSupport()) {
        this->recordOp(GrDiscardOp::Make(renderTargetContext->accessRenderTarget()),
                       renderTargetContext);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetOpList::copySurface(GrSurface* dst,
                                       GrSurface* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }
#ifdef ENABLE_MDB
    this->addDependency(src);
#endif

    // Copy surface doesn't work through a GrGpuCommandBuffer. By passing nullptr for the context we
    // force this to occur between command buffers and execute directly on GrGpu. This workaround
    // goes away with MDB.
    this->recordOp(std::move(op), nullptr);
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

GrOp* GrRenderTargetOpList::recordOp(std::unique_ptr<GrOp> op,
                                     GrRenderTargetContext* renderTargetContext,
                                     const SkRect& clippedBounds) {
    GrRenderTarget* renderTarget =
            renderTargetContext ? renderTargetContext->accessRenderTarget()
                                : nullptr;

    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), renderTarget->uniqueID());
    GrOP_INFO("Recording (%s, B%u)\n"
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
    // If we don't have a valid destination render target then we cannot reorder.
    if (maxCandidates && renderTarget) {
        int i = 0;
        while (true) {
            const RecordedOp& candidate = fRecordedOps.fromBack(i);
            // We cannot continue to search backwards if the render target changes
            if (candidate.fRenderTarget.get() != renderTarget) {
                GrOP_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            if (candidate.fOp->combineIfPossible(op.get(), *this->caps())) {
                GrOP_INFO("\t\tCombining with (%s, B%u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                GrOP_INFO("\t\t\tCombined op info:\n");
                GrOP_INFO(SkTabString(candidate.fOp->dumpInfo(), 4).c_str());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, candidate.fOp.get(), op.get());
                join(&fRecordedOps.fromBack(i).fClippedBounds,
                     fRecordedOps.fromBack(i).fClippedBounds, clippedBounds);
                return candidate.fOp.get();
            }
            // Stop going backwards if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedOps.fromBack(i).fClippedBounds;
            if (!can_reorder(candidateBounds, clippedBounds)) {
                GrOP_INFO("\t\tIntersects with (%s, B%u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
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
    GR_AUDIT_TRAIL_OP_RESULT_NEW(fAuditTrail, op);
    fRecordedOps.emplace_back(std::move(op), clippedBounds, renderTarget);
    fLastFullClearOp = nullptr;
    fLastFullClearRenderTargetID.makeInvalid();
    return fRecordedOps.back().fOp.get();
}

void GrRenderTargetOpList::forwardCombine() {
    if (fMaxOpLookahead <= 0) {
        return;
    }
    for (int i = 0; i < fRecordedOps.count() - 2; ++i) {
        GrOp* op = fRecordedOps[i].fOp.get();
        GrRenderTarget* renderTarget = fRecordedOps[i].fRenderTarget.get();
        // If we don't have a valid destination render target ID then we cannot reorder.
        if (!renderTarget) {
            continue;
        }
        const SkRect& opBounds = fRecordedOps[i].fClippedBounds;
        int maxCandidateIdx = SkTMin(i + fMaxOpLookahead, fRecordedOps.count() - 1);
        int j = i + 1;
        while (true) {
            const RecordedOp& candidate = fRecordedOps[j];
            // We cannot continue to search if the render target changes
            if (candidate.fRenderTarget.get() != renderTarget) {
                GrOP_INFO("\t\tBreaking because of (%s, B%u) Rendertarget\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            if (j == i +1) {
                // We assume op would have combined with candidate when the candidate was added
                // via backwards combining in recordOp.

                // not sure why this fires with device-clipping in gm/complexclip4.cpp
//                SkASSERT(!op->combineIfPossible(candidate.fOp.get(), *this->caps()));

            } else if (op->combineIfPossible(candidate.fOp.get(), *this->caps())) {
                GrOP_INFO("\t\tCombining with (%s, B%u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, op, candidate.fOp.get());
                fRecordedOps[j].fOp = std::move(fRecordedOps[i].fOp);
                join(&fRecordedOps[j].fClippedBounds, fRecordedOps[j].fClippedBounds, opBounds);
                break;
            }
            // Stop going traversing if we would cause a painter's order violation.
            const SkRect& candidateBounds = fRecordedOps[j].fClippedBounds;
            if (!can_reorder(candidateBounds, opBounds)) {
                GrOP_INFO("\t\tIntersects with (%s, B%u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            ++j;
            if (j > maxCandidateIdx) {
                GrOP_INFO("\t\tReached max lookahead or end of op array %d\n", i);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetOpList::clearStencilClip(const GrFixedClip& clip,
                                            bool insideStencilMask,
                                            GrRenderTargetContext* renderTargetContext) {
    this->recordOp(GrClearStencilClipOp::Make(clip, insideStencilMask,
                                              renderTargetContext->accessRenderTarget()),
                   renderTargetContext);
}
