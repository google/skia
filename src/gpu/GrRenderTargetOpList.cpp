/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <SkRectPriv.h>
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

////////////////////////////////////////////////////////////////////////////////

static inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

////////////////////////////////////////////////////////////////////////////////

GrRenderTargetOpList::OpChain::OpChain(std::unique_ptr<GrOp> op, GrAppliedClip* appliedClip,
                                       const DstProxy* dstProxy)
        : fHead(std::move(op)), fTail(fHead.get()), fAppliedClip(appliedClip) {
    if (dstProxy) {
        fDstProxy = *dstProxy;
    }
    fBounds = fHead->bounds();
}

void GrRenderTargetOpList::OpChain::visitProxies(const GrOp::VisitProxyFunc& func, GrOp::VisitorType visitor) const {
    if (!fHead) {
        SkASSERT(!fTail);
        return;
    }
    for (const auto& op : GrOp::ChainRange<>(fHead.get())) {
        op.visitProxies(func, visitor);
    }
    if (fDstProxy.proxy()) {
        func(fDstProxy.proxy());
    }
    if (fAppliedClip) {
        fAppliedClip->visitProxies(func);
    }
}

void GrRenderTargetOpList::OpChain::deleteOps(GrOpMemoryPool* pool) {
    while (fHead) {
        std::unique_ptr<GrOp> nextHead = fHead->cutChain();
        pool->release(std::move(fHead));
        fHead = std::move(nextHead);
    }
    fTail = nullptr;
}
using DstProxy = GrXferProcessor::DstProxy;

// Assumes that we know that chains a and b are chainable.
static std::tuple<std::unique_ptr<GrOp>, GrOp*>
        join_chains(std::unique_ptr<GrOp> aHead, GrOp* aTail, std::unique_ptr<GrOp> bHead, GrOp* bTail,
                    const GrCaps& caps, GrOpMemoryPool* pool) {
    do {
        bool merged = false;
        SkRect bounds = SkRectPriv::MakeLargestInverted();
        GrOp* a = aTail;
        while (a) {
            bool canForwardMerge = (a == aTail) || can_reorder(a->bounds(), bounds);
            bool canBackwardMerge = (a == aTail) || can_reorder(bHead->bounds(), bounds);
            // Ugh we'll go all the way to the head of a every time.
            if (canForwardMerge || canBackwardMerge) {
                auto result = a->combineIfPossible(bHead.get(), caps);
                SkASSERT(result != GrOp::CombineResult::kCannotCombine);
                merged = (result == GrOp::CombineResult::kMerged);
            }
            if (merged) {
                // We prefer backward merging.
                if (canBackwardMerge) {
                    auto temp = std::move(bHead);
                    bHead = temp->cutChain();
                    if (bHead) { SkDEBUGCODE(bHead->validateChain(bTail)); }
                    pool->release(std::move(temp));
                } else {
                    // We merged the contents of bHead into a. We will replace bHead with a in
                    // chain b.
                    SkASSERT(canForwardMerge);
                    std::unique_ptr<GrOp> detachedA;
                    if (a == aHead.get()) {
                        detachedA = std::move(aHead);
                        aHead = detachedA->cutChain();
                        if (aTail == detachedA.get()) {
                            aTail = nullptr;
                        }
                    } else {
                        GrOp* prev = a->prevInChain();
                        detachedA = prev->cutChain();
                        if (auto aNext = detachedA->cutChain()) {
                            prev->chainConcat(std::move(aNext));
                        } else {
                            SkASSERT(aTail == a);
                            aTail = prev;
                        }
                    }
                    if (auto bNext = bHead->cutChain()) {
                        detachedA->chainConcat(std::move(bNext));
                    } else {
                        bTail = nullptr;
                    }
                    pool->release(std::move(bHead));
                    bHead = std::move(detachedA);
                    if (!bTail) {
                        bTail = bHead.get();
                    }
                    SkDEBUGCODE(bHead->validateChain(bTail));
                    SkASSERT(SkToBool(aHead) == SkToBool(aTail));
                    if (!aHead) {
                        // We merged all the nodes in chain a to chain b.
                        return {std::move(bHead), bTail};
                    }
                }
                break;
            } else {
                bounds.joinPossiblyEmptyRect(a->bounds());
                a = a->prevInChain();
            }
        }
        // If we weren't able to merge bHead then pop bHead from chain b and make it the new tail
        // of a.
        if (!merged) {
            aTail->chainConcat(std::move(bHead));
            // There is a slight inefficiency here now that aTail is pointing to a node that came
            // from b. Presumably, we already tried to merge nodes from b into the new aTail and
            // yet our loop starts from aTail and moves to aHead when attempting to find a merge
            // for bHead. To address this we'd have to keep track of an original aTail which
            // would move forward if aTail merges back into bHead and also ensure we do bounds
            // checks against the nodes at the end of chain a even if we don't merge into them.
            // Keeping it a little simpler for now.
            aTail = aTail->nextInChain();
            bHead = aTail->cutChain();
            if (bHead) { SkDEBUGCODE(bHead->validateChain(bTail)); }

        }
        SkDEBUGCODE(aHead->validateChain(aTail));
        if (bHead) { SkDEBUGCODE(bHead->validateChain(bTail)); }
    } while (bHead);
    return {std::move(aHead), aTail};
}

static std::tuple<std::unique_ptr<GrOp>, GrOp*, std::unique_ptr<GrOp>, GrOp*>
concat_chains(std::unique_ptr<GrOp> aHead, GrOp* aTail, const DstProxy& dstProxyA,
              const GrAppliedClip* appliedClipA,
              std::unique_ptr<GrOp> bHead, GrOp* bTail, const DstProxy& dstProxyB,
              const GrAppliedClip* appliedClipB,
              const GrCaps& caps, GrOpMemoryPool* pool) {
    SkASSERT(aHead);
    SkASSERT(aTail);
    SkASSERT(bHead);
    if (aHead->classID() != bHead->classID()) {
        return {std::move(aHead), aTail, std::move(bHead), bTail};
    }
    if (SkToBool(appliedClipA) != SkToBool(appliedClipB)) {
        return {std::move(aHead), aTail, std::move(bHead), bTail};
    }
    if (appliedClipA && *appliedClipA != *appliedClipB) {
        return {std::move(aHead), aTail, std::move(bHead), bTail};
    }
    if (SkToBool(dstProxyA.proxy()) != SkToBool(dstProxyB.proxy())) {
        return {std::move(aHead), aTail, std::move(bHead), bTail};
    }
    if (dstProxyA.proxy() && dstProxyA != dstProxyB) {
        return {std::move(aHead), aTail, std::move(bHead), bTail};
    }
    SkDEBUGCODE(bool first = true;)
    do {
        switch (aTail->combineIfPossible(bHead.get(), caps)) {
            case GrOp::CombineResult::kCannotCombine:
                // If an op supports chaining then it is required that chaining is transitive and
                // that if any two ops in two different chains can merge then the two chains
                // may also be chained together. Thus, we should only hit this on the first
                // iteration.
                SkASSERT(first);
                return {std::move(aHead), aTail, std::move(bHead), bTail};
            case GrOp::CombineResult::kMayChain:
                std::tie(aHead, aTail) = join_chains(std::move(aHead), aTail, std::move(bHead), bTail, caps, pool);
                return {std::move(aHead), aTail, nullptr, nullptr};
            case GrOp::CombineResult::kMerged: {
                auto newHead = bHead->cutChain();
                pool->release(std::move(bHead));
                bHead = std::move(newHead);
                break;
            }
        }
        SkDEBUGCODE(first = false);
    } while (bHead);
    SkDEBUGCODE(aHead->validateChain(aTail));
    // All the ops from chain b merged.
    return {std::move(aHead), aTail, nullptr, nullptr};
}

bool GrRenderTargetOpList::OpChain::prependChain(OpChain* that, const GrCaps& caps,
                                                 GrOpMemoryPool* pool) {
    std::tie(that->fHead, that->fTail, fHead, fTail) =
            concat_chains(std::move(that->fHead), that->fTail, that->dstProxy(),
                                that->appliedClip(),
                                std::move(fHead), fTail, this->dstProxy(), this->appliedClip(),
                                caps, pool);
    if (fHead) {
        SkDEBUGCODE(fHead->validateChain(fTail));
        // append failed
        return false;
    }
    // 'that' owns the combined chain. Move it into this.
    fHead = std::move(that->fHead);
    fTail = that->fTail;
    SkDEBUGCODE(fHead->validateChain(fTail));
    that->fTail = nullptr;
    fBounds.joinPossiblyEmptyRect(that->fBounds);

    that->fDstProxy.setProxy(nullptr);
    if (that->fAppliedClip) {
        for (int i = 0; i < that->fAppliedClip->numClipCoverageFragmentProcessors(); ++i) {
            that->fAppliedClip->detachClipCoverageFragmentProcessor(i);
        }
    }
    return true;
}

std::unique_ptr<GrOp> GrRenderTargetOpList::OpChain::appendOp(std::unique_ptr<GrOp> op,
                                                              const DstProxy* dstProxy,
                                                              const GrAppliedClip* applidClip,
                                                              const GrCaps& caps,
                                                              GrOpMemoryPool* pool) {
    const GrXferProcessor::DstProxy noDstProxy;
    if (!dstProxy) {
        dstProxy = &noDstProxy;
    }
    SkASSERT(op->isChainTail() && op->isChainTail());
    SkASSERT(op.get() != fTail);
    GrOp* tail = op.get();
    SkRect opBounds = op->bounds();
    std::tie(fHead, fTail, op, std::ignore) =
            concat_chains(std::move(fHead), fTail, this->dstProxy(), fAppliedClip, std::move(op),
                    tail, *dstProxy, applidClip, caps, pool);
    SkDEBUGCODE(fHead->validateChain(fTail));
    if (op) {
        // append failed, give the op back to the caller.
        return op;
    }
    fBounds.joinPossiblyEmptyRect(opBounds);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

GrRenderTargetOpList::GrRenderTargetOpList(GrResourceProvider* resourceProvider,
                                           sk_sp<GrOpMemoryPool> opMemoryPool,
                                           GrRenderTargetProxy* proxy,
                                           GrAuditTrail* auditTrail)
        : INHERITED(resourceProvider, std::move(opMemoryPool), proxy, auditTrail)
        , fLastClipStackGenID(SK_InvalidUniqueID)
        SkDEBUGCODE(, fNumClips(0)) {
}

void GrRenderTargetOpList::deleteOps() {
    for (auto& chain : fOpChains) {
        chain.deleteOps(fOpMemoryPool.get());
    }
    fOpChains.reset();
}

GrRenderTargetOpList::~GrRenderTargetOpList() {
    this->deleteOps();
}

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void GrRenderTargetOpList::dump(bool printDependencies) const {
    INHERITED::dump(printDependencies);

    SkDebugf("ops (%d):\n", fOpChains.count());
    for (int i = 0; i < fOpChains.count(); ++i) {
        SkDebugf("*******************************\n");
        if (!fOpChains[i].head()) {
            SkDebugf("%d: <combined forward or failed instantiation>\n", i);
        } else {
            SkDebugf("%d: %s\n", i, fOpChains[i].head()->name());
            SkRect bounds = fOpChains[i].bounds();
            SkDebugf("ClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", bounds.fLeft,
                     bounds.fTop, bounds.fRight, bounds.fBottom);
            for (const auto& op : GrOp::ChainRange<>(fOpChains[i].head())) {
                SkString info = SkTabString(op.dumpInfo(), 1);
                SkDebugf("%s\n", info.c_str());
                bounds = op.bounds();
                SkDebugf("\tClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", bounds.fLeft,
                         bounds.fTop, bounds.fRight, bounds.fBottom);
            }
        }
    }
}

void GrRenderTargetOpList::visitProxies_debugOnly(const GrOp::VisitProxyFunc& func) const {
    for (const OpChain& chain : fOpChains) {
        chain.visitProxies(func, GrOp::VisitorType::kOther);
    }
}

static void assert_chain_bounds(const GrOp* op, const SkRect bounds) {
    SkASSERT(op->isChainHead());
    while ((op = op->nextInChain())) {
        SkASSERT(bounds.contains(op->bounds()));
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
    for (const auto& chain : fOpChains) {
        if (chain.head()) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
            TRACE_EVENT0("skia", fOpChains[i].fOp->name());
#endif
            GrOpFlushState::OpArgs opArgs = {
                chain.head(),
                fTarget.get()->asRenderTargetProxy(),
                chain.appliedClip(),
                chain.dstProxy()
            };
            SkDEBUGCODE(assert_chain_bounds(chain.head(), chain.bounds()));
            flushState->setOpArgs(&opArgs);
            chain.head()->prepare(flushState);
            flushState->setOpArgs(nullptr);
        }
    }
}

static GrGpuRTCommandBuffer* create_command_buffer(GrGpu* gpu,
                                                   GrRenderTarget* rt,
                                                   GrSurfaceOrigin origin,
                                                   const SkRect& bounds,
                                                   GrLoadOp colorLoadOp,
                                                   const SkPMColor4f& loadClearColor,
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

    return gpu->getCommandBuffer(rt, origin, bounds, kColorLoadStoreInfo, stencilLoadAndStoreInfo);
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
    if (0 == fOpChains.count() && GrLoadOp::kClear != fColorLoadOp &&
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
                                                    fTarget.get()->getBoundsRect(),
                                                    fColorLoadOp,
                                                    fLoadClearColor,
                                                    fStencilLoadOp);
    flushState->setCommandBuffer(commandBuffer);
    commandBuffer->begin();

    // Draw all the generated geometry.
    for (const auto& chain : fOpChains) {
        if (!chain.head()) {
            continue;
        }
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        TRACE_EVENT0("skia", fOpChains[i].fOp->name());
#endif

        GrOpFlushState::OpArgs opArgs {
            chain.head(),
            fTarget.get()->asRenderTargetProxy(),
            chain.appliedClip(),
            chain.dstProxy()
        };

        flushState->setOpArgs(&opArgs);
        chain.head()->execute(flushState);
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

void GrRenderTargetOpList::fullClear(GrContext* context, const SkPMColor4f& color) {

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
    for (OpChain& recordedOp : fOpChains) {
        hasUninstantiatedProxy = false;
        recordedOp.visitProxies(checkInstantiation, GrOp::VisitorType::kOther);
        if (hasUninstantiatedProxy) {
            // When instantiation of the proxy fails we drop the Op
            recordedOp.deleteOps(fOpMemoryPool.get());
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
    if (fOpChains.count()) {
        alloc->addInterval(fTarget.get(), cur, cur+fOpChains.count()-1);
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
    for (const OpChain& recordedOp : fOpChains) {
        // only diff from the GrTextureOpList version
        recordedOp.visitProxies(gather, GrOp::VisitorType::kAllocatorGather);

        // Even though the op may have been moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

void GrRenderTargetOpList::recordOp(std::unique_ptr<GrOp> op,
                                    const GrCaps& caps,
                                    GrAppliedClip* clip,
                                    const DstProxy* dstProxy) {
    SkDEBUGCODE(op->validate();)
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
    int maxCandidates = SkTMin(kMaxOpLookback, fOpChains.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            OpChain& candidate = fOpChains.fromBack(i);
            op = candidate.appendOp(std::move(op), dstProxy, clip, caps, fOpMemoryPool.get());
            if (!op) {
                SkDEBUGCODE(assert_chain_bounds(candidate.head(), candidate.bounds());)
                return;
            }
            // Stop going backwards if we would cause a painter's order violation.
            if (!can_reorder(candidate.bounds(), op->bounds())) {
                GrOP_INFO("\t\tBackward: Intersects with chain (%s, head opID: %u)\n",
                        candidate.head()->name(), candidate.head()->uniqueID());
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
    fOpChains.emplace_back(std::move(op), clip, dstProxy);
}

void GrRenderTargetOpList::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());
    GrOP_INFO("opList: %d ForwardCombine %d ops:\n", this->uniqueID(), fOpChains.count());

    for (int i = 0; i < fOpChains.count() - 1; ++i) {
        OpChain& chain = fOpChains[i];
        int maxCandidateIdx = SkTMin(i + kMaxOpLookahead, fOpChains.count() - 1);
        int j = i + 1;
        while (true) {
            OpChain& candidate = fOpChains[j];
            if (candidate.prependChain(&chain, caps, fOpMemoryPool.get())) {
                SkDEBUGCODE(assert_chain_bounds(candidate.head(), candidate.bounds());)
                break;
            }
            // Stop traversing if we would cause a painter's order violation.
            if (!can_reorder(chain.bounds(), candidate.bounds())) {
                GrOP_INFO("\t\t%d: chain (%s head opID: %u) -> "
                          "Intersects with chain (%s, head opID: %u)\n",
                          i, chain.head()->name(), chain.head()->uniqueID(),
                          candidate.head()->name(), candidate.head()->uniqueID());
                break;
            }
            if (++j > maxCandidateIdx) {
                GrOP_INFO("\t\t%d: chain (%s opID: %u) -> Reached max lookahead or end of array\n", i,
                          chain.head()->name(), chain.head()->uniqueID());
                break;
            }
        }
    }
}

