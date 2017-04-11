/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetOpList.h"
#include "GrAuditTrail.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "ops/GrClearOp.h"
#include "ops/GrCopySurfaceOp.h"
#include "ops/GrDiscardOp.h"
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
        , fLastClipStackGenID(SK_InvalidUniqueID)
        , fClipAllocator(fClipAllocatorStorage, sizeof(fClipAllocatorStorage),
                         sizeof(fClipAllocatorStorage)) {

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
            const SkRect& bounds = fRecordedOps[i].fOp->bounds();
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", bounds.fLeft,
                     bounds.fTop, bounds.fRight, bounds.fBottom);
        }
    }
}

void GrRenderTargetOpList::validateTargetsSingleRenderTarget() const {
    GrRenderTarget* rt = nullptr;
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i].fOp) {
            continue;       // combined forward
        }

        if (!rt) {
            rt = fRecordedOps[i].fRenderTarget.get();
        } else {
            SkASSERT(fRecordedOps[i].fRenderTarget.get() == rt);
        }
    }
}
#endif

void GrRenderTargetOpList::prepareOps(GrOpFlushState* flushState) {
    // MDB TODO: add SkASSERT(this->isClosed());

    // Loop over the ops that haven't yet been prepared.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i].fOp) {
            GrOpFlushState::DrawOpArgs opArgs;
            if (fRecordedOps[i].fRenderTarget) {
                opArgs = {
                    fRecordedOps[i].fRenderTarget.get(),
                    fRecordedOps[i].fAppliedClip,
                    fRecordedOps[i].fDstTexture
                };
            }
            flushState->setDrawOpArgs(&opArgs);
            fRecordedOps[i].fOp->prepare(flushState);
            flushState->setDrawOpArgs(nullptr);
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
        GrOpFlushState::DrawOpArgs opArgs;
        if (fRecordedOps[i].fRenderTarget) {
            opArgs = {
                fRecordedOps[i].fRenderTarget.get(),
                fRecordedOps[i].fAppliedClip,
                fRecordedOps[i].fDstTexture
            };
            flushState->setDrawOpArgs(&opArgs);
        }
        fRecordedOps[i].fOp->execute(flushState);
        flushState->setDrawOpArgs(nullptr);
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
    fLastFullClearResourceID.makeInvalid();
    fLastFullClearProxyID.makeInvalid();
    fRecordedOps.reset();
    if (fInstancedRendering) {
        fInstancedRendering->endFlush();
    }
}

void GrRenderTargetOpList::abandonGpuResources() {
    if (GrCaps::InstancedSupport::kNone != this->caps()->instancedSupport()) {
        InstancedRendering* ir = this->instancedRendering();
        ir->resetGpuResources(InstancedRendering::ResetType::kAbandon);
    }
}

void GrRenderTargetOpList::freeGpuResources() {
    if (GrCaps::InstancedSupport::kNone != this->caps()->instancedSupport()) {
        InstancedRendering* ir = this->instancedRendering();
        ir->resetGpuResources(InstancedRendering::ResetType::kDestroy);
    }
}

void GrRenderTargetOpList::fullClear(GrRenderTargetContext* renderTargetContext, GrColor color) {
    // MDB TODO: remove this. Right now we need the renderTargetContext for the
    // accessRenderTarget call. This method should just take the renderTargetProxy.
    GrRenderTarget* renderTarget = renderTargetContext->accessRenderTarget();
    if (!renderTarget) {
        return;
    }

    // Currently this just inserts or updates the last clear op. However, once in MDB this can
    // remove all the previously recorded ops and change the load op to clear with supplied
    // color.
    // TODO: this needs to be updated to use GrSurfaceProxy::UniqueID
    SkASSERT((fLastFullClearResourceID == renderTarget->uniqueID()) ==
             (fLastFullClearProxyID == renderTargetContext->asRenderTargetProxy()->uniqueID()));
    if (fLastFullClearResourceID == renderTarget->uniqueID()) {
        // As currently implemented, fLastFullClearOp should be the last op because we would
        // have cleared it when another op was recorded.
        SkASSERT(fRecordedOps.back().fOp.get() == fLastFullClearOp);
        fLastFullClearOp->setColor(color);
        return;
    }
    std::unique_ptr<GrClearOp> op(GrClearOp::Make(GrFixedClip::Disabled(), color,
                                                  renderTargetContext));
    if (!op) {
        return;
    }
    if (GrOp* clearOp = this->recordOp(std::move(op), renderTargetContext)) {
        // This is either the clear op we just created or another one that it combined with.
        fLastFullClearOp = static_cast<GrClearOp*>(clearOp);
        fLastFullClearResourceID = renderTarget->uniqueID();
        fLastFullClearProxyID = renderTargetContext->asRenderTargetProxy()->uniqueID();
    }
}

void GrRenderTargetOpList::discard(GrRenderTargetContext* renderTargetContext) {
    // Currently this just inserts a discard op. However, once in MDB this can remove all the
    // previously recorded ops and change the load op to discard.
    if (this->caps()->discardRenderTargetSupport()) {
        std::unique_ptr<GrOp> op(GrDiscardOp::Make(renderTargetContext));
        if (!op) {
            return;
        }
        this->recordOp(std::move(op), renderTargetContext);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetOpList::copySurface(GrResourceProvider* resourceProvider,
                                       GrSurfaceProxy* dst,
                                       GrSurfaceProxy* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(resourceProvider, dst, src, srcRect, dstPoint);
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

bool GrRenderTargetOpList::combineIfPossible(const RecordedOp& a, GrOp* b,
                                             const GrAppliedClip* bClip,
                                             const DstTexture* bDstTexture) {
    if (a.fAppliedClip) {
        if (!bClip) {
            return false;
        }
        if (*a.fAppliedClip != *bClip) {
            return false;
        }
    } else if (bClip) {
        return false;
    }
    if (bDstTexture) {
        if (a.fDstTexture != *bDstTexture) {
            return false;
        }
    } else if (a.fDstTexture.texture()) {
        return false;
    }
    return a.fOp->combineIfPossible(b, *this->caps());
}

GrOp* GrRenderTargetOpList::recordOp(std::unique_ptr<GrOp> op,
                                     GrRenderTargetContext* renderTargetContext,
                                     GrAppliedClip* clip,
                                     const DstTexture* dstTexture) {
    GrRenderTarget* renderTarget =
            renderTargetContext ? renderTargetContext->accessRenderTarget()
                                : nullptr;

    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), renderTarget->uniqueID(),
                          renderTargetContext->asRenderTargetProxy()->uniqueID());
    GrOP_INFO("Recording (%s, opID: %u)\n"
              "\tBounds: [L: %f T: %f R: %f B: %f]\n",
               op->name(),
               op->uniqueID(),
               op->bounds().fLeft, op->bounds().fTop,
               op->bounds().fRight, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GrOP_INFO("\tClipped Bounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", op->bounds().fLeft,
              op->bounds().fTop, op->bounds().fRight, op->bounds().fBottom);
    GrOP_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(fMaxOpLookback, fRecordedOps.count());
    // If we don't have a valid destination render target then we cannot reorder.
    if (maxCandidates && renderTarget) {
        int i = 0;
        while (true) {
            const RecordedOp& candidate = fRecordedOps.fromBack(i);
            // We cannot continue to search backwards if the render target changes
            if (candidate.fRenderTarget.get() != renderTarget) {
                GrOP_INFO("\t\tBreaking because of (%s, opID: %u) Rendertarget mismatch\n",
                          candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            if (this->combineIfPossible(candidate, op.get(), clip, dstTexture)) {
                GrOP_INFO("\t\tCombining with (%s, opID: %u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                GrOP_INFO("\t\t\tCombined op info:\n");
                GrOP_INFO(SkTabString(candidate.fOp->dumpInfo(), 4).c_str());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, candidate.fOp.get(), op.get());
                return candidate.fOp.get();
            }
            // Stop going backwards if we would cause a painter's order violation.
            if (!can_reorder(fRecordedOps.fromBack(i).fOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\tIntersects with (%s, opID: %u)\n", candidate.fOp->name(),
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
    if (clip) {
        clip = fClipAllocator.make<GrAppliedClip>(std::move(*clip));
    }
    fRecordedOps.emplace_back(std::move(op), renderTarget, clip, dstTexture);
    fRecordedOps.back().fOp->wasRecorded();
    fLastFullClearOp = nullptr;
    fLastFullClearResourceID.makeInvalid();
    fLastFullClearProxyID.makeInvalid();
    return fRecordedOps.back().fOp.get();
}

void GrRenderTargetOpList::forwardCombine() {
    if (fMaxOpLookahead <= 0) {
        return;
    }
    for (int i = 0; i < fRecordedOps.count() - 1; ++i) {
        GrOp* op = fRecordedOps[i].fOp.get();
        GrRenderTarget* renderTarget = fRecordedOps[i].fRenderTarget.get();
        // If we don't have a valid destination render target ID then we cannot reorder.
        if (!renderTarget) {
            continue;
        }
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
            if (this->combineIfPossible(fRecordedOps[i], candidate.fOp.get(),
                                        candidate.fAppliedClip, &candidate.fDstTexture)) {
                GrOP_INFO("\t\tCombining with (%s, B%u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, op, candidate.fOp.get());
                fRecordedOps[j].fOp = std::move(fRecordedOps[i].fOp);
                break;
            }
            // Stop going traversing if we would cause a painter's order violation.
            if (!can_reorder(fRecordedOps[j].fOp->bounds(), op->bounds())) {
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

