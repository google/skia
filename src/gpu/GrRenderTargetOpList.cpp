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
#include "GrMemoryPool.h"
#include "GrRect.h"
#include "GrRenderTargetContext.h"
#include "GrResourceAllocator.h"
#include "ops/GrClearOp.h"
#include "ops/GrCopySurfaceOp.h"
#include "SkTraceEvent.h"


////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kMaxOpLookback = 10;
static const int kMaxOpLookahead = 10;

GrRenderTargetOpList::GrRenderTargetOpList(GrResourceProvider* resourceProvider,
                                           sk_sp<GrOpMemoryPool> opMemoryPool,
                                           GrRenderTargetProxy* proxy,
                                           GrAuditTrail* auditTrail)
        : INHERITED(resourceProvider, std::move(opMemoryPool), proxy, auditTrail)
        , fLastClipStackGenID(SK_InvalidUniqueID)
        SkDEBUGCODE(, fNumClips(0)) {
}

void GrRenderTargetOpList::RecordedOp::deleteOp(GrOpMemoryPool* opMemoryPool) {
    opMemoryPool->release(std::move(fOp));
}

void GrRenderTargetOpList::deleteOps() {
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i].fOp) {
            fRecordedOps[i].deleteOp(fOpMemoryPool.get());
        }
    }
    fRecordedOps.reset();
}

GrRenderTargetOpList::~GrRenderTargetOpList() {
    this->deleteOps();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrRenderTargetOpList::dump(bool printDependencies) const {
    INHERITED::dump(printDependencies);

    SkDebugf("ops (%d):\n", fRecordedOps.count());
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fRecordedOps[i].fOp) {
            SkDebugf("%d: <combined forward or failed instantiation>\n", i);
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

void GrRenderTargetOpList::visitProxies_debugOnly(const GrOp::VisitProxyFunc& func) const {
    for (const RecordedOp& recordedOp : fRecordedOps) {
        recordedOp.visitProxies(func);
    }
}

static void assert_chain_bounds(const GrOp* op) {
    SkASSERT(op->isChainHead());
    auto headBounds = op->bounds();
    while ((op = op->nextInChain())) {
        SkASSERT(headBounds.contains(op->bounds()));
    }
}
#endif

void GrRenderTargetOpList::onPrepare(GrOpFlushState* flushState) {
    SkASSERT(fTarget.get()->peekRenderTarget());
    SkASSERT(this->isClosed());
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    TRACE_EVENT0("skia", TRACE_FUNC);
#endif

    // Loop over the ops that haven't yet been prepared.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (fRecordedOps[i].fOp && fRecordedOps[i].fOp->isChainHead()) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
            TRACE_EVENT0("skia", fRecordedOps[i].fOp->name());
#endif
            GrOpFlushState::OpArgs opArgs = {
                fRecordedOps[i].fOp.get(),
                fTarget.get()->asRenderTargetProxy(),
                fRecordedOps[i].fAppliedClip,
                fRecordedOps[i].fDstProxy
            };
            SkDEBUGCODE(assert_chain_bounds(opArgs.fOp));
            flushState->setOpArgs(&opArgs);
            fRecordedOps[i].fOp->prepare(flushState);
            flushState->setOpArgs(nullptr);
        }
    }
}

static GrGpuRTCommandBuffer* create_command_buffer(GrGpu* gpu,
                                                   GrRenderTarget* rt,
                                                   GrSurfaceOrigin origin,
                                                   GrLoadOp colorLoadOp,
                                                   GrColor loadClearColor,
                                                   GrLoadOp stencilLoadOp) {
    const GrGpuRTCommandBuffer::LoadAndStoreInfo kColorLoadStoreInfo {
        colorLoadOp,
        GrStoreOp::kStore,
        loadClearColor
    };

    // TODO:
    // We would like to (at this level) only ever clear & discard. We would need
    // to stop splitting up higher level opLists for copyOps to achieve that.
    // Note: we would still need SB loads and stores but they would happen at a
    // lower level (inside the VK command buffer).
    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo stencilLoadAndStoreInfo {
        stencilLoadOp,
        GrStoreOp::kStore,
    };

    return gpu->getCommandBuffer(rt, origin, kColorLoadStoreInfo, stencilLoadAndStoreInfo);
}

// TODO: this is where GrOp::renderTarget is used (which is fine since it
// is at flush time). However, we need to store the RenderTargetProxy in the
// Ops and instantiate them here.
bool GrRenderTargetOpList::onExecute(GrOpFlushState* flushState) {
    // TODO: Forcing the execution of the discard here isn't ideal since it will cause us to do a
    // discard and then store the data back in memory so that the load op on future draws doesn't
    // think the memory is unitialized. Ideally we would want a system where we are tracking whether
    // the proxy itself has valid data or not, and then use that as a signal on whether we should be
    // loading or discarding. In that world we wouldni;t need to worry about executing oplists with
    // no ops just to do a discard.
    if (0 == fRecordedOps.count() && GrLoadOp::kClear != fColorLoadOp &&
        GrLoadOp::kDiscard != fColorLoadOp) {
        return false;
    }

    SkASSERT(fTarget.get()->peekRenderTarget());
    TRACE_EVENT0("skia", TRACE_FUNC);

    // TODO: at the very least, we want the stencil store op to always be discard (at this
    // level). In Vulkan, sub-command buffers would still need to load & store the stencil buffer.
    GrGpuRTCommandBuffer* commandBuffer = create_command_buffer(
                                                    flushState->gpu(),
                                                    fTarget.get()->peekRenderTarget(),
                                                    fTarget.get()->origin(),
                                                    fColorLoadOp,
                                                    fLoadClearColor,
                                                    fStencilLoadOp);
    flushState->setCommandBuffer(commandBuffer);
    commandBuffer->begin();

    // Draw all the generated geometry.
    for (int i = 0; i < fRecordedOps.count(); ++i) {
        if (!fRecordedOps[i].fOp || !fRecordedOps[i].fOp->isChainHead()) {
            continue;
        }
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        TRACE_EVENT0("skia", fRecordedOps[i].fOp->name());
#endif

        GrOpFlushState::OpArgs opArgs {
            fRecordedOps[i].fOp.get(),
            fTarget.get()->asRenderTargetProxy(),
            fRecordedOps[i].fAppliedClip,
            fRecordedOps[i].fDstProxy
        };

        flushState->setOpArgs(&opArgs);
        fRecordedOps[i].fOp->execute(flushState);
        flushState->setOpArgs(nullptr);
    }

    commandBuffer->end();
    flushState->gpu()->submit(commandBuffer);
    flushState->setCommandBuffer(nullptr);

    return true;
}

void GrRenderTargetOpList::endFlush() {
    fLastClipStackGenID = SK_InvalidUniqueID;
    this->deleteOps();
    fClipAllocator.reset();
    INHERITED::endFlush();
}

void GrRenderTargetOpList::discard() {
    // Discard calls to in-progress opLists are ignored. Calls at the start update the
    // opLists' color & stencil load ops.
    if (this->isEmpty()) {
        fColorLoadOp = GrLoadOp::kDiscard;
        fStencilLoadOp = GrLoadOp::kDiscard;
    }
}

void GrRenderTargetOpList::fullClear(GrContext* context, GrColor color) {

    // This is conservative. If the opList is marked as needing a stencil buffer then there
    // may be a prior op that writes to the stencil buffer. Although the clear will ignore the
    // stencil buffer, following draw ops may not so we can't get rid of all the preceding ops.
    // Beware! If we ever add any ops that have a side effect beyond modifying the stencil
    // buffer we will need a more elaborate tracking system (skbug.com/7002).
    if (this->isEmpty() || !fTarget.get()->asRenderTargetProxy()->needsStencil()) {
        this->deleteOps();
        fDeferredProxies.reset();
        fColorLoadOp = GrLoadOp::kClear;
        fLoadClearColor = color;
        return;
    }

    std::unique_ptr<GrClearOp> op(GrClearOp::Make(context, GrFixedClip::Disabled(),
                                                  color, fTarget.get()));
    if (!op) {
        return;
    }

    this->recordOp(std::move(op), *context->contextPriv().caps());
}

////////////////////////////////////////////////////////////////////////////////

// This closely parallels GrTextureOpList::copySurface but renderTargetOpLists
// also store the applied clip and dest proxy with the op
bool GrRenderTargetOpList::copySurface(GrContext* context,
                                       GrSurfaceProxy* dst,
                                       GrSurfaceProxy* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    SkASSERT(dst->asRenderTargetProxy() == fTarget.get());
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(context, dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }

    this->addOp(std::move(op), *context->contextPriv().caps());
    return true;
}

void GrRenderTargetOpList::purgeOpsWithUninstantiatedProxies() {
    bool hasUninstantiatedProxy = false;
    auto checkInstantiation = [&hasUninstantiatedProxy](GrSurfaceProxy* p) {
        if (!p->isInstantiated()) {
            hasUninstantiatedProxy = true;
        }
    };
    for (RecordedOp& recordedOp : fRecordedOps) {
        hasUninstantiatedProxy = false;
        if (recordedOp.fOp) {
            recordedOp.visitProxies(checkInstantiation);
        }
        if (hasUninstantiatedProxy) {
            // When instantiation of the proxy fails we drop the Op
            recordedOp.deleteOp(fOpMemoryPool.get());
        }
    }
}

void GrRenderTargetOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    unsigned int cur = alloc->numOps();

    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        SkASSERT(!fDeferredProxies[i]->isInstantiated());
        // We give all the deferred proxies a write usage at the very start of flushing. This
        // locks them out of being reused for the entire flush until they are read - and then
        // they can be recycled. This is a bit unfortunate because a flush can proceed in waves
        // with sub-flushes. The deferred proxies only need to be pinned from the start of
        // the sub-flush in which they appear.
        alloc->addInterval(fDeferredProxies[i], 0, 0);
    }

    // Add the interval for all the writes to this opList's target
    if (fRecordedOps.count()) {
        alloc->addInterval(fTarget.get(), cur, cur+fRecordedOps.count()-1);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(fTarget.get());
        alloc->incOps();
    }

    auto gather = [ alloc SkDEBUGCODE(, this) ] (GrSurfaceProxy* p) {
        alloc->addInterval(p SkDEBUGCODE(, fTarget.get() == p));
    };
    for (const RecordedOp& recordedOp : fRecordedOps) {
        recordedOp.visitProxies(gather); // only diff from the GrTextureOpList version

        // Even though the op may have been moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

static inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

GrOp::CombineResult GrRenderTargetOpList::combineIfPossible(const RecordedOp& a, GrOp* b,
                                                            const GrAppliedClip* bClip,
                                                            const DstProxy* bDstProxy,
                                                            const GrCaps& caps) {
    if (a.fAppliedClip) {
        if (!bClip) {
            return GrOp::CombineResult::kCannotCombine;
        }
        if (*a.fAppliedClip != *bClip) {
            return GrOp::CombineResult::kCannotCombine;
        }
    } else if (bClip) {
        return GrOp::CombineResult::kCannotCombine;
    }
    if (bDstProxy) {
        if (a.fDstProxy != *bDstProxy) {
            return GrOp::CombineResult::kCannotCombine;
        }
    } else if (a.fDstProxy.proxy()) {
        return GrOp::CombineResult::kCannotCombine;
    }
    return a.fOp->combineIfPossible(b, caps);
}

uint32_t GrRenderTargetOpList::recordOp(std::unique_ptr<GrOp> op,
                                        const GrCaps& caps,
                                        GrAppliedClip* clip,
                                        const DstProxy* dstProxy) {
    SkASSERT(fTarget.get());

    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), fTarget.get()->uniqueID());
    GrOP_INFO("opList: %d Recording (%s, opID: %u)\n"
              "\tBounds [L: %.2f, T: %.2f R: %.2f B: %.2f]\n",
               this->uniqueID(),
               op->name(),
               op->uniqueID(),
               op->bounds().fLeft, op->bounds().fTop,
               op->bounds().fRight, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GrOP_INFO("\tOutcome:\n");
    int maxCandidates = SkTMin(kMaxOpLookback, fRecordedOps.count());
    int firstChainableIdx = -1;
    if (maxCandidates) {
        int i = 0;
        while (true) {
            const RecordedOp& candidate = fRecordedOps.fromBack(i);
            auto combineResult = this->combineIfPossible(candidate, op.get(), clip, dstProxy, caps);
            switch (combineResult) {
                case GrOp::CombineResult::kMayChain:
                    if (candidate.fOp->isChainTail() && firstChainableIdx < 0) {
                        GrOP_INFO("\t\tBackward: Can chain with (%s, opID: %u)\n",
                                  candidate.fOp->name(), candidate.fOp->uniqueID());
                        firstChainableIdx = i;
                    }
                    break;
                case GrOp::CombineResult::kMerged:
                    GrOP_INFO("\t\tBackward: Combining with (%s, opID: %u)\n",
                              candidate.fOp->name(), candidate.fOp->uniqueID());
                    GrOP_INFO("\t\t\tBackward: Combined op info:\n");
                    GrOP_INFO(SkTabString(candidate.fOp->dumpInfo(), 4).c_str());
                    GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, candidate.fOp.get(), op.get());
                    fOpMemoryPool->release(std::move(op));
                    return SK_InvalidUniqueID;
                case GrOp::CombineResult::kCannotCombine:
                    break;
            }
            // Stop going backwards if we would cause a painter's order violation. We only need to
            // test against chain heads as elements of a chain always draw in their chain head's
            // slot.
            if (candidate.fOp->isChainHead() &&
                !can_reorder(candidate.fOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\tBackward: Intersects with (%s, opID: %u)\n", candidate.fOp->name(),
                          candidate.fOp->uniqueID());
                break;
            }
            ++i;
            if (i == maxCandidates) {
                GrOP_INFO("\t\tBackward: Reached max lookback or beginning of op array %d\n", i);
                break;
            }
        }
    } else {
        GrOP_INFO("\t\tBackward: FirstOp\n");
    }
    GR_AUDIT_TRAIL_OP_RESULT_NEW(fAuditTrail, op);
    if (clip) {
        clip = fClipAllocator.make<GrAppliedClip>(std::move(*clip));
        SkDEBUGCODE(fNumClips++;)
    }
    if (firstChainableIdx >= 0) {
        // If we chain this op it will draw in the slot of the head of the chain. We have to check
        // that the new op's bounds don't intersect any of the other ops between firstChainableIdx
        // and the head of that op's chain. We only need to test against chain heads as elements of
        // a chain always draw in their chain head's slot.
        const GrOp* chainHead = fRecordedOps.fromBack(firstChainableIdx).fOp->chainHead();
        int idx = firstChainableIdx;
        bool chain = true;
        while (fRecordedOps.fromBack(idx).fOp.get() != chainHead) {
            // If idx is not in the same chain then we have to check against its bounds as we will
            // draw before it (when chainHead draws).
            const GrOp* testOp = fRecordedOps.fromBack(idx).fOp.get();
            if (testOp->isChainHead() && !can_reorder(testOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\tBackward: Intersects with (%s, opID: %u). Cannot chain.\n",
                          testOp->name(), testOp->uniqueID());
                chain = false;
                break;
            }
            ++idx;
            // We must encounter the chain head before running off the beginning of the list.
            SkASSERT(idx < fRecordedOps.count());
        }
        if (chain) {
            GrOp* prevOp = fRecordedOps.fromBack(firstChainableIdx).fOp.get();
            GrOP_INFO("\t\t\tBackward: Chained to (%s, opID: %u)\n", prevOp->name(),
                      prevOp->uniqueID());
            prevOp->setNextInChain(op.get());
        }
    }
    fRecordedOps.emplace_back(std::move(op), clip, dstProxy);
    return this->uniqueID();
}

void GrRenderTargetOpList::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());

    GrOP_INFO("opList: %d ForwardCombine %d ops:\n", this->uniqueID(), fRecordedOps.count());

    for (int i = 0; i < fRecordedOps.count() - 1; ++i) {
        GrOp* op = fRecordedOps[i].fOp.get();

        int maxCandidateIdx = SkTMin(i + kMaxOpLookahead, fRecordedOps.count() - 1);
        int j = i + 1;
        int firstChainableIdx = -1;
        while (true) {
            const RecordedOp& candidate = fRecordedOps[j];
            auto combineResult =
                    this->combineIfPossible(fRecordedOps[i], candidate.fOp.get(),
                                            candidate.fAppliedClip, &candidate.fDstProxy, caps);
            switch (combineResult) {
                case GrOp::CombineResult::kMayChain:
                    if (firstChainableIdx < 0 && !fRecordedOps[i].fOp->isChained() &&
                        !fRecordedOps[j].fOp->isChained()) {
                        GrOP_INFO("\t\tForward: Can chain with (%s, opID: %u)\n",
                                  candidate.fOp->name(), candidate.fOp->uniqueID());
                        firstChainableIdx = j;
                    }
                    break;
                case GrOp::CombineResult::kMerged:
                    GrOP_INFO("\t\t%d: (%s opID: %u) -> Combining with (%s, opID: %u)\n", i,
                              op->name(), op->uniqueID(), candidate.fOp->name(),
                              candidate.fOp->uniqueID());
                    GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(fAuditTrail, op, candidate.fOp.get());
                    fOpMemoryPool->release(std::move(fRecordedOps[j].fOp));
                    fRecordedOps[j].fOp = std::move(fRecordedOps[i].fOp);
                    break;
                case GrOp::CombineResult::kCannotCombine:
                    break;
            }
            if (!fRecordedOps[i].fOp) {
                break;
            }
            // Stop traversing if we would cause a painter's order violation.
            if (candidate.fOp->isChainHead() &&
                !can_reorder(candidate.fOp->bounds(), op->bounds())) {
                GrOP_INFO("\t\t%d: (%s opID: %u) -> Intersects with (%s, opID: %u)\n",
                          i, op->name(), op->uniqueID(),
                          candidate.fOp->name(), candidate.fOp->uniqueID());
                break;
            }
            ++j;
            if (j > maxCandidateIdx) {
                if (firstChainableIdx >= 0) {
                    GrOp* nextOp = fRecordedOps[firstChainableIdx].fOp.get();
                    GrOP_INFO("\t\t\tForward: Chained to (%s, opID: %u)\n", nextOp->name(),
                              nextOp->uniqueID());
                    // We have to chain i before firstChainableIdx in order to preserve their
                    // relative order as they may overlap.
                    fRecordedOps[i].fOp->setNextInChain(nextOp);
                    // However we want to draw them *after* any ops that occur between them. So move
                    // the head of the new chain to the later slot as we only execute chain heads.
                    std::swap(fRecordedOps[i].fOp, fRecordedOps[firstChainableIdx].fOp);
                } else {
                    GrOP_INFO("\t\t%d: (%s opID: %u) -> Reached max lookahead or end of array\n", i,
                              op->name(), op->uniqueID());
                }
                break;
            }
        }
    }
}

