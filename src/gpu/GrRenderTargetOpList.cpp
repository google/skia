/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrAuditTrail.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkExchange.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRect.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetOpList.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/ops/GrClearOp.h"
#include "src/gpu/ops/GrCopySurfaceOp.h"

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kMaxOpMergeDistance = 10;
static const int kMaxOpChainDistance = 10;

////////////////////////////////////////////////////////////////////////////////

using DstProxy = GrXferProcessor::DstProxy;

////////////////////////////////////////////////////////////////////////////////

static inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

////////////////////////////////////////////////////////////////////////////////

inline GrRenderTargetOpList::OpChain::List::List(std::unique_ptr<GrOp> op)
        : fHead(std::move(op)), fTail(fHead.get()) {
    this->validate();
}

inline GrRenderTargetOpList::OpChain::List::List(List&& that) { *this = std::move(that); }

inline GrRenderTargetOpList::OpChain::List& GrRenderTargetOpList::OpChain::List::operator=(
        List&& that) {
    fHead = std::move(that.fHead);
    fTail = that.fTail;
    that.fTail = nullptr;
    this->validate();
    return *this;
}

inline std::unique_ptr<GrOp> GrRenderTargetOpList::OpChain::List::popHead() {
    SkASSERT(fHead);
    auto temp = fHead->cutChain();
    std::swap(temp, fHead);
    if (!fHead) {
        SkASSERT(fTail == temp.get());
        fTail = nullptr;
    }
    return temp;
}

inline std::unique_ptr<GrOp> GrRenderTargetOpList::OpChain::List::removeOp(GrOp* op) {
#ifdef SK_DEBUG
    auto head = op;
    while (head->prevInChain()) { head = head->prevInChain(); }
    SkASSERT(head == fHead.get());
#endif
    auto prev = op->prevInChain();
    if (!prev) {
        SkASSERT(op == fHead.get());
        return this->popHead();
    }
    auto temp = prev->cutChain();
    if (auto next = temp->cutChain()) {
        prev->chainConcat(std::move(next));
    } else {
        SkASSERT(fTail == op);
        fTail = prev;
    }
    this->validate();
    return temp;
}

inline void GrRenderTargetOpList::OpChain::List::pushHead(std::unique_ptr<GrOp> op) {
    SkASSERT(op);
    SkASSERT(op->isChainHead());
    SkASSERT(op->isChainTail());
    if (fHead) {
        op->chainConcat(std::move(fHead));
        fHead = std::move(op);
    } else {
        fHead = std::move(op);
        fTail = fHead.get();
    }
}

inline void GrRenderTargetOpList::OpChain::List::pushTail(std::unique_ptr<GrOp> op) {
    SkASSERT(op->isChainTail());
    fTail->chainConcat(std::move(op));
    fTail = fTail->nextInChain();
}

inline void GrRenderTargetOpList::OpChain::List::validate() const {
#ifdef SK_DEBUG
    if (fHead) {
        SkASSERT(fTail);
        fHead->validateChain(fTail);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrRenderTargetOpList::OpChain::OpChain(std::unique_ptr<GrOp> op,
                                       GrProcessorSet::Analysis processorAnalysis,
                                       GrAppliedClip* appliedClip, const DstProxy* dstProxy)
        : fList{std::move(op)}
        , fProcessorAnalysis(processorAnalysis)
        , fAppliedClip(appliedClip) {
    if (fProcessorAnalysis.requiresDstTexture()) {
        SkASSERT(dstProxy && dstProxy->proxy());
        fDstProxy = *dstProxy;
    }
    fBounds = fList.head()->bounds();
}

void GrRenderTargetOpList::OpChain::visitProxies(const GrOp::VisitProxyFunc& func,
                                                 GrOp::VisitorType visitor) const {
    if (fList.empty()) {
        return;
    }
    for (const auto& op : GrOp::ChainRange<>(fList.head())) {
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
    while (!fList.empty()) {
        pool->release(fList.popHead());
    }
}

// Concatenates two op chains and attempts to merge ops across the chains. Assumes that we know that
// the two chains are chainable. Returns the new chain.
GrRenderTargetOpList::OpChain::List GrRenderTargetOpList::OpChain::DoConcat(
        List chainA, List chainB, const GrCaps& caps, GrOpMemoryPool* pool,
        GrAuditTrail* auditTrail) {
    // We process ops in chain b from head to tail. We attempt to merge with nodes in a, starting
    // at chain a's tail and working toward the head. We produce one of the following outcomes:
    // 1) b's head is merged into an op in a.
    // 2) An op from chain a is merged into b's head. (In this case b's head gets processed again.)
    // 3) b's head is popped from chain a and added at the tail of a.
    // After result 3 we don't want to attempt to merge the next head of b with the new tail of a,
    // as we assume merges were already attempted when chain b was created. So we keep track of the
    // original tail of a and start our iteration of a there. We also track the bounds of the nodes
    // appended to chain a that will be skipped for bounds testing. If the original tail of a is
    // merged into an op in b (case 2) then we advance the "original tail" towards the head of a.
    GrOp* origATail = chainA.tail();
    SkRect skipBounds = SkRectPriv::MakeLargestInverted();
    do {
        int numMergeChecks = 0;
        bool merged = false;
        bool noSkip = (origATail == chainA.tail());
        SkASSERT(noSkip == (skipBounds == SkRectPriv::MakeLargestInverted()));
        bool canBackwardMerge = noSkip || can_reorder(chainB.head()->bounds(), skipBounds);
        SkRect forwardMergeBounds = skipBounds;
        GrOp* a = origATail;
        while (a) {
            bool canForwardMerge =
                    (a == chainA.tail()) || can_reorder(a->bounds(), forwardMergeBounds);
            if (canForwardMerge || canBackwardMerge) {
                auto result = a->combineIfPossible(chainB.head(), caps);
                SkASSERT(result != GrOp::CombineResult::kCannotCombine);
                merged = (result == GrOp::CombineResult::kMerged);
                GrOP_INFO("\t\t: (%s opID: %u) -> Combining with (%s, opID: %u)\n",
                          chainB.head()->name(), chainB.head()->uniqueID(), a->name(),
                          a->uniqueID());
            }
            if (merged) {
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(auditTrail, a, chainB.head());
                if (canBackwardMerge) {
                    pool->release(chainB.popHead());
                } else {
                    // We merged the contents of b's head into a. We will replace b's head with a in
                    // chain b.
                    SkASSERT(canForwardMerge);
                    if (a == origATail) {
                        origATail = a->prevInChain();
                    }
                    std::unique_ptr<GrOp> detachedA = chainA.removeOp(a);
                    pool->release(chainB.popHead());
                    chainB.pushHead(std::move(detachedA));
                    if (chainA.empty()) {
                        // We merged all the nodes in chain a to chain b.
                        return chainB;
                    }
                }
                break;
            } else {
                if (++numMergeChecks == kMaxOpMergeDistance) {
                    break;
                }
                forwardMergeBounds.joinNonEmptyArg(a->bounds());
                canBackwardMerge =
                        canBackwardMerge && can_reorder(chainB.head()->bounds(), a->bounds());
                a = a->prevInChain();
            }
        }
        // If we weren't able to merge b's head then pop b's head from chain b and make it the new
        // tail of a.
        if (!merged) {
            chainA.pushTail(chainB.popHead());
            skipBounds.joinNonEmptyArg(chainA.tail()->bounds());
        }
    } while (!chainB.empty());
    return chainA;
}

// Attempts to concatenate the given chain onto our own and merge ops across the chains. Returns
// whether the operation succeeded. On success, the provided list will be returned empty.
bool GrRenderTargetOpList::OpChain::tryConcat(
        List* list, GrProcessorSet::Analysis processorAnalysis, const DstProxy& dstProxy,
        const GrAppliedClip* appliedClip, const SkRect& bounds, const GrCaps& caps,
        GrOpMemoryPool* pool, GrAuditTrail* auditTrail) {
    SkASSERT(!fList.empty());
    SkASSERT(!list->empty());
    SkASSERT(fProcessorAnalysis.requiresDstTexture() == SkToBool(fDstProxy.proxy()));
    SkASSERT(processorAnalysis.requiresDstTexture() == SkToBool(dstProxy.proxy()));
    // All returns use explicit tuple constructor rather than {a, b} to work around old GCC bug.
    if (fList.head()->classID() != list->head()->classID() ||
        SkToBool(fAppliedClip) != SkToBool(appliedClip) ||
        (fAppliedClip && *fAppliedClip != *appliedClip) ||
        (fProcessorAnalysis.requiresNonOverlappingDraws() !=
                processorAnalysis.requiresNonOverlappingDraws()) ||
        (fProcessorAnalysis.requiresNonOverlappingDraws() &&
                // Non-overlaping draws are only required when Ganesh will either insert a barrier,
                // or read back a new dst texture between draws. In either case, we can neither
                // chain nor combine overlapping Ops.
                GrRectsTouchOrOverlap(fBounds, bounds)) ||
        (fProcessorAnalysis.requiresDstTexture() != processorAnalysis.requiresDstTexture()) ||
        (fProcessorAnalysis.requiresDstTexture() && fDstProxy != dstProxy)) {
        return false;
    }

    SkDEBUGCODE(bool first = true;)
    do {
        switch (fList.tail()->combineIfPossible(list->head(), caps)) {
            case GrOp::CombineResult::kCannotCombine:
                // If an op supports chaining then it is required that chaining is transitive and
                // that if any two ops in two different chains can merge then the two chains
                // may also be chained together. Thus, we should only hit this on the first
                // iteration.
                SkASSERT(first);
                return false;
            case GrOp::CombineResult::kMayChain:
                fList = DoConcat(std::move(fList), skstd::exchange(*list, List()), caps, pool,
                                 auditTrail);
                // The above exchange cleared out 'list'. The list needs to be empty now for the
                // loop to terminate.
                SkASSERT(list->empty());
                break;
            case GrOp::CombineResult::kMerged: {
                GrOP_INFO("\t\t: (%s opID: %u) -> Combining with (%s, opID: %u)\n",
                          list->tail()->name(), list->tail()->uniqueID(), list->head()->name(),
                          list->head()->uniqueID());
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(auditTrail, fList.tail(), list->head());
                pool->release(list->popHead());
                break;
            }
        }
        SkDEBUGCODE(first = false);
    } while (!list->empty());

    // The new ops were successfully merged and/or chained onto our own.
    fBounds.joinPossiblyEmptyRect(bounds);
    return true;
}

bool GrRenderTargetOpList::OpChain::prependChain(OpChain* that, const GrCaps& caps,
                                                 GrOpMemoryPool* pool, GrAuditTrail* auditTrail) {
    if (!that->tryConcat(
            &fList, fProcessorAnalysis, fDstProxy, fAppliedClip, fBounds, caps, pool, auditTrail)) {
        this->validate();
        // append failed
        return false;
    }

    // 'that' owns the combined chain. Move it into 'this'.
    SkASSERT(fList.empty());
    fList = std::move(that->fList);
    fBounds = that->fBounds;

    that->fDstProxy.setProxy(nullptr);
    if (that->fAppliedClip) {
        for (int i = 0; i < that->fAppliedClip->numClipCoverageFragmentProcessors(); ++i) {
            that->fAppliedClip->detachClipCoverageFragmentProcessor(i);
        }
    }
    this->validate();
    return true;
}

std::unique_ptr<GrOp> GrRenderTargetOpList::OpChain::appendOp(
        std::unique_ptr<GrOp> op, GrProcessorSet::Analysis processorAnalysis,
        const DstProxy* dstProxy, const GrAppliedClip* appliedClip, const GrCaps& caps,
        GrOpMemoryPool* pool, GrAuditTrail* auditTrail) {
    const GrXferProcessor::DstProxy noDstProxy;
    if (!dstProxy) {
        dstProxy = &noDstProxy;
    }
    SkASSERT(op->isChainHead() && op->isChainTail());
    SkRect opBounds = op->bounds();
    List chain(std::move(op));
    if (!this->tryConcat(
            &chain, processorAnalysis, *dstProxy, appliedClip, opBounds, caps, pool, auditTrail)) {
        // append failed, give the op back to the caller.
        this->validate();
        return chain.popHead();
    }

    SkASSERT(chain.empty());
    this->validate();
    return nullptr;
}

inline void GrRenderTargetOpList::OpChain::validate() const {
#ifdef SK_DEBUG
    fList.validate();
    for (const auto& op : GrOp::ChainRange<>(fList.head())) {
        // Not using SkRect::contains because we allow empty rects.
        SkASSERT(fBounds.fLeft <= op.bounds().fLeft && fBounds.fTop <= op.bounds().fTop &&
                 fBounds.fRight >= op.bounds().fRight && fBounds.fBottom >= op.bounds().fBottom);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrRenderTargetOpList::GrRenderTargetOpList(sk_sp<GrOpMemoryPool> opMemoryPool,
                                           sk_sp<GrRenderTargetProxy> proxy,
                                           GrAuditTrail* auditTrail)
        : INHERITED(std::move(opMemoryPool), std::move(proxy), auditTrail)
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
            TRACE_EVENT0("skia", chain.head()->name());
#endif
            GrOpFlushState::OpArgs opArgs = {
                chain.head(),
                fTarget.get()->asRenderTargetProxy(),
                chain.appliedClip(),
                chain.dstProxy()
            };
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
    if (fOpChains.empty() && GrLoadOp::kClear != fColorLoadOp &&
        GrLoadOp::kDiscard != fColorLoadOp) {
        return false;
    }

    SkASSERT(fTarget.get()->peekRenderTarget());
    TRACE_EVENT0("skia", TRACE_FUNC);

    // TODO: at the very least, we want the stencil store op to always be discard (at this
    // level). In Vulkan, sub-command buffers would still need to load & store the stencil buffer.

    // Make sure load ops are not kClear if the GPU needs to use draws for clears
    SkASSERT(fColorLoadOp != GrLoadOp::kClear ||
             !flushState->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(fStencilLoadOp != GrLoadOp::kClear ||
             !flushState->gpu()->caps()->performStencilClearsAsDraws());
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
        TRACE_EVENT0("skia", chain.head()->name());
#endif

        GrOpFlushState::OpArgs opArgs {
            chain.head(),
            fTarget.get()->asRenderTargetProxy(),
            chain.appliedClip(),
            chain.dstProxy()
        };

        flushState->setOpArgs(&opArgs);
        chain.head()->execute(flushState, chain.bounds());
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

void GrRenderTargetOpList::setStencilLoadOp(GrLoadOp op) {
    fStencilLoadOp = op;
}

void GrRenderTargetOpList::setColorLoadOp(GrLoadOp op, const SkPMColor4f& color) {
    fColorLoadOp = op;
    fLoadClearColor = color;
}

bool GrRenderTargetOpList::resetForFullscreenClear() {
    // Mark the color load op as discard (this may be followed by a clearColorOnLoad call to make
    // the load op kClear, or it may be followed by an explicit op). In the event of an absClear()
    // after a regular clear(), we could end up with a clear load op and a real clear op in the list
    // if the load op were not reset here.
    fColorLoadOp = GrLoadOp::kDiscard;

    // Regardless of how the clear is implemented (native clear or a fullscreen quad), all prior ops
    // would be overwritten, so discard them entirely. The one exception is if the opList is marked
    // as needing a stencil buffer then there may be a prior op that writes to the stencil buffer.
    // Although the clear will ignore the stencil buffer, following draw ops may not so we can't get
    // rid of all the preceding ops. Beware! If we ever add any ops that have a side effect beyond
    // modifying the stencil buffer we will need a more elaborate tracking system (skbug.com/7002).
    // Additionally, if we previously recorded a wait op, we cannot delete the wait op. Until we
    // track the wait ops separately from normal ops, we have to avoid clearing out any ops.
    if (this->isEmpty() || (!fTarget.get()->asRenderTargetProxy()->needsStencil() && !fHasWaitOp)) {
        this->deleteOps();
        fDeferredProxies.reset();

        // If the opList is using a render target which wraps a vulkan command buffer, we can't do a
        // clear load since we cannot change the render pass that we are using. Thus we fall back to
        // making a clear op in this case.
        return !fTarget.get()->asRenderTargetProxy()->wrapsVkSecondaryCB();
    }

    // Could not empty the list, so an op must be added to handle the clear
    return false;
}

////////////////////////////////////////////////////////////////////////////////

// This closely parallels GrTextureOpList::copySurface but renderTargetOpLists
// also store the applied clip and dest proxy with the op
bool GrRenderTargetOpList::copySurface(GrRecordingContext* context,
                                       GrSurfaceProxy* dst,
                                       GrSurfaceProxy* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint) {
    SkASSERT(dst->asRenderTargetProxy() == fTarget.get());
    std::unique_ptr<GrOp> op = GrCopySurfaceOp::Make(context, dst, src, srcRect, dstPoint);
    if (!op) {
        return false;
    }

    this->addOp(std::move(op), *context->priv().caps());
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

bool GrRenderTargetOpList::onIsUsed(GrSurfaceProxy* proxyToCheck) const {
    bool used = false;

    auto visit = [ proxyToCheck, &used ] (GrSurfaceProxy* p) {
        if (p == proxyToCheck) {
            used = true;
        }
    };
    for (const OpChain& recordedOp : fOpChains) {
        recordedOp.visitProxies(visit, GrOp::VisitorType::kOther);
    }

    return used;
}

void GrRenderTargetOpList::gatherProxyIntervals(GrResourceAllocator* alloc) const {

    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        SkASSERT(!fDeferredProxies[i]->isInstantiated());
        // We give all the deferred proxies a write usage at the very start of flushing. This
        // locks them out of being reused for the entire flush until they are read - and then
        // they can be recycled. This is a bit unfortunate because a flush can proceed in waves
        // with sub-flushes. The deferred proxies only need to be pinned from the start of
        // the sub-flush in which they appear.
        alloc->addInterval(fDeferredProxies[i], 0, 0, GrResourceAllocator::ActualUse::kNo);
    }

    // Add the interval for all the writes to this opList's target
    if (fOpChains.count()) {
        unsigned int cur = alloc->curOp();

        alloc->addInterval(fTarget.get(), cur, cur + fOpChains.count() - 1,
                           GrResourceAllocator::ActualUse::kYes);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(fTarget.get(), alloc->curOp(), alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes);
        alloc->incOps();
    }

    auto gather = [ alloc SkDEBUGCODE(, this) ] (GrSurfaceProxy* p) {
        alloc->addInterval(p, alloc->curOp(), alloc->curOp(), GrResourceAllocator::ActualUse::kYes
                           SkDEBUGCODE(, fTarget.get() == p));
    };
    for (const OpChain& recordedOp : fOpChains) {
        // only diff from the GrTextureOpList version
        recordedOp.visitProxies(gather, GrOp::VisitorType::kAllocatorGather);

        // Even though the op may have been (re)moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

void GrRenderTargetOpList::recordOp(
        std::unique_ptr<GrOp> op, GrProcessorSet::Analysis processorAnalysis, GrAppliedClip* clip,
        const DstProxy* dstProxy, const GrCaps& caps) {
    SkDEBUGCODE(op->validate();)
    SkASSERT(processorAnalysis.requiresDstTexture() == (dstProxy && dstProxy->proxy()));
    SkASSERT(fTarget.get());

    // A closed GrOpList should never receive new/more ops
    SkASSERT(!this->isClosed());
    if (!op->bounds().isFinite()) {
        fOpMemoryPool->release(std::move(op));
        return;
    }

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
    int maxCandidates = SkTMin(kMaxOpChainDistance, fOpChains.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            OpChain& candidate = fOpChains.fromBack(i);
            op = candidate.appendOp(std::move(op), processorAnalysis, dstProxy, clip, caps,
                                    fOpMemoryPool.get(), fAuditTrail);
            if (!op) {
                return;
            }
            // Stop going backwards if we would cause a painter's order violation.
            if (!can_reorder(candidate.bounds(), op->bounds())) {
                GrOP_INFO("\t\tBackward: Intersects with chain (%s, head opID: %u)\n",
                          candidate.head()->name(), candidate.head()->uniqueID());
                break;
            }
            if (++i == maxCandidates) {
                GrOP_INFO("\t\tBackward: Reached max lookback or beginning of op array %d\n", i);
                break;
            }
        }
    } else {
        GrOP_INFO("\t\tBackward: FirstOp\n");
    }
    if (clip) {
        clip = fClipAllocator.make<GrAppliedClip>(std::move(*clip));
        SkDEBUGCODE(fNumClips++;)
    }
    fOpChains.emplace_back(std::move(op), processorAnalysis, clip, dstProxy);
}

void GrRenderTargetOpList::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());
    GrOP_INFO("opList: %d ForwardCombine %d ops:\n", this->uniqueID(), fOpChains.count());

    for (int i = 0; i < fOpChains.count() - 1; ++i) {
        OpChain& chain = fOpChains[i];
        int maxCandidateIdx = SkTMin(i + kMaxOpChainDistance, fOpChains.count() - 1);
        int j = i + 1;
        while (true) {
            OpChain& candidate = fOpChains[j];
            if (candidate.prependChain(&chain, caps, fOpMemoryPool.get(), fAuditTrail)) {
                break;
            }
            // Stop traversing if we would cause a painter's order violation.
            if (!can_reorder(chain.bounds(), candidate.bounds())) {
                GrOP_INFO(
                        "\t\t%d: chain (%s head opID: %u) -> "
                        "Intersects with chain (%s, head opID: %u)\n",
                        i, chain.head()->name(), chain.head()->uniqueID(), candidate.head()->name(),
                        candidate.head()->uniqueID());
                break;
            }
            if (++j > maxCandidateIdx) {
                GrOP_INFO("\t\t%d: chain (%s opID: %u) -> Reached max lookahead or end of array\n",
                          i, chain.head()->name(), chain.head()->uniqueID());
                break;
            }
        }
    }
}

