/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/OpsTask.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/base/SkScopeExit.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/geometry/GrRect.h"

using namespace skia_private;

////////////////////////////////////////////////////////////////////////////////

namespace {

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kMaxOpMergeDistance = 10;
static const int kMaxOpChainDistance = 10;

////////////////////////////////////////////////////////////////////////////////

inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

GrOpsRenderPass* create_render_pass(GrGpu* gpu,
                                    GrRenderTarget* rt,
                                    bool useMSAASurface,
                                    GrAttachment* stencil,
                                    GrSurfaceOrigin origin,
                                    const SkIRect& bounds,
                                    GrLoadOp colorLoadOp,
                                    const std::array<float, 4>& loadClearColor,
                                    GrLoadOp stencilLoadOp,
                                    GrStoreOp stencilStoreOp,
                                    const TArray<GrSurfaceProxy*, true>& sampledProxies,
                                    GrXferBarrierFlags renderPassXferBarriers) {
    const GrOpsRenderPass::LoadAndStoreInfo kColorLoadStoreInfo {
        colorLoadOp,
        GrStoreOp::kStore,
        loadClearColor
    };

    // TODO:
    // We would like to (at this level) only ever clear & discard. We would need
    // to stop splitting up higher level OpsTasks for copyOps to achieve that.
    // Note: we would still need SB loads and stores but they would happen at a
    // lower level (inside the VK command buffer).
    const GrOpsRenderPass::StencilLoadAndStoreInfo stencilLoadAndStoreInfo {
        stencilLoadOp,
        stencilStoreOp,
    };

    return gpu->getOpsRenderPass(rt, useMSAASurface, stencil, origin, bounds, kColorLoadStoreInfo,
                                 stencilLoadAndStoreInfo, sampledProxies, renderPassXferBarriers);
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

namespace skgpu::ganesh {

inline OpsTask::OpChain::List::List(GrOp::Owner op)
        : fHead(std::move(op)), fTail(fHead.get()) {
    this->validate();
}

inline OpsTask::OpChain::List::List(List&& that) { *this = std::move(that); }

inline OpsTask::OpChain::List& OpsTask::OpChain::List::operator=(List&& that) {
    fHead = std::move(that.fHead);
    fTail = that.fTail;
    that.fTail = nullptr;
    this->validate();
    return *this;
}

inline GrOp::Owner OpsTask::OpChain::List::popHead() {
    SkASSERT(fHead);
    auto temp = fHead->cutChain();
    std::swap(temp, fHead);
    if (!fHead) {
        SkASSERT(fTail == temp.get());
        fTail = nullptr;
    }
    return temp;
}

inline GrOp::Owner OpsTask::OpChain::List::removeOp(GrOp* op) {
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

inline void OpsTask::OpChain::List::pushHead(GrOp::Owner op) {
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

inline void OpsTask::OpChain::List::pushTail(GrOp::Owner op) {
    SkASSERT(op->isChainTail());
    fTail->chainConcat(std::move(op));
    fTail = fTail->nextInChain();
}

inline void OpsTask::OpChain::List::validate() const {
#ifdef SK_DEBUG
    if (fHead) {
        SkASSERT(fTail);
        fHead->validateChain(fTail);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

OpsTask::OpChain::OpChain(GrOp::Owner op, GrProcessorSet::Analysis processorAnalysis,
                          GrAppliedClip* appliedClip, const GrDstProxyView* dstProxyView)
        : fList{std::move(op)}
        , fProcessorAnalysis(processorAnalysis)
        , fAppliedClip(appliedClip) {
    if (fProcessorAnalysis.requiresDstTexture()) {
        SkASSERT(dstProxyView && dstProxyView->proxy());
        fDstProxyView = *dstProxyView;
    }
    fBounds = fList.head()->bounds();
}

void OpsTask::OpChain::visitProxies(const GrVisitProxyFunc& func) const {
    if (fList.empty()) {
        return;
    }
    for (const auto& op : GrOp::ChainRange<>(fList.head())) {
        op.visitProxies(func);
    }
    if (fDstProxyView.proxy()) {
        func(fDstProxyView.proxy(), skgpu::Mipmapped::kNo);
    }
    if (fAppliedClip) {
        fAppliedClip->visitProxies(func);
    }
}

void OpsTask::OpChain::deleteOps() {
    while (!fList.empty()) {
        // Since the value goes out of scope immediately, the GrOp::Owner deletes the op.
        fList.popHead();
    }
}

// Concatenates two op chains and attempts to merge ops across the chains. Assumes that we know that
// the two chains are chainable. Returns the new chain.
OpsTask::OpChain::List OpsTask::OpChain::DoConcat(List chainA, List chainB, const GrCaps& caps,
                                                  SkArenaAlloc* opsTaskArena,
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
                auto result = a->combineIfPossible(chainB.head(), opsTaskArena, caps);
                SkASSERT(result != GrOp::CombineResult::kCannotCombine);
                merged = (result == GrOp::CombineResult::kMerged);
                GrOP_INFO("\t\t: (%s opID: %u) -> Combining with (%s, opID: %u)\n",
                          chainB.head()->name(), chainB.head()->uniqueID(), a->name(),
                          a->uniqueID());
            }
            if (merged) {
                GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(auditTrail, a, chainB.head());
                if (canBackwardMerge) {
                    // The GrOp::Owner releases the op.
                    chainB.popHead();
                } else {
                    // We merged the contents of b's head into a. We will replace b's head with a in
                    // chain b.
                    SkASSERT(canForwardMerge);
                    if (a == origATail) {
                        origATail = a->prevInChain();
                    }
                    GrOp::Owner detachedA = chainA.removeOp(a);
                    // The GrOp::Owner releases the op.
                    chainB.popHead();
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
bool OpsTask::OpChain::tryConcat(
        List* list, GrProcessorSet::Analysis processorAnalysis, const GrDstProxyView& dstProxyView,
        const GrAppliedClip* appliedClip, const SkRect& bounds, const GrCaps& caps,
        SkArenaAlloc* opsTaskArena, GrAuditTrail* auditTrail) {
    SkASSERT(!fList.empty());
    SkASSERT(!list->empty());
    SkASSERT(fProcessorAnalysis.requiresDstTexture() == SkToBool(fDstProxyView.proxy()));
    SkASSERT(processorAnalysis.requiresDstTexture() == SkToBool(dstProxyView.proxy()));
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
        (fProcessorAnalysis.requiresDstTexture() && fDstProxyView != dstProxyView)) {
        return false;
    }

    SkDEBUGCODE(bool first = true;)
    do {
        switch (fList.tail()->combineIfPossible(list->head(), opsTaskArena, caps))
        {
            case GrOp::CombineResult::kCannotCombine:
                // If an op supports chaining then it is required that chaining is transitive and
                // that if any two ops in two different chains can merge then the two chains
                // may also be chained together. Thus, we should only hit this on the first
                // iteration.
                SkASSERT(first);
                return false;
            case GrOp::CombineResult::kMayChain:
                fList = DoConcat(std::move(fList), std::exchange(*list, List()), caps, opsTaskArena,
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
                // The GrOp::Owner releases the op.
                list->popHead();
                break;
            }
        }
        SkDEBUGCODE(first = false);
    } while (!list->empty());

    // The new ops were successfully merged and/or chained onto our own.
    fBounds.joinPossiblyEmptyRect(bounds);
    return true;
}

bool OpsTask::OpChain::prependChain(OpChain* that, const GrCaps& caps, SkArenaAlloc* opsTaskArena,
                                    GrAuditTrail* auditTrail) {
    if (!that->tryConcat(&fList, fProcessorAnalysis, fDstProxyView, fAppliedClip, fBounds, caps,
                         opsTaskArena, auditTrail)) {
        this->validate();
        // append failed
        return false;
    }

    // 'that' owns the combined chain. Move it into 'this'.
    SkASSERT(fList.empty());
    fList = std::move(that->fList);
    fBounds = that->fBounds;

    that->fDstProxyView.setProxyView({});
    if (that->fAppliedClip && that->fAppliedClip->hasCoverageFragmentProcessor()) {
        // Obliterates the processor.
        that->fAppliedClip->detachCoverageFragmentProcessor();
    }
    this->validate();
    return true;
}

GrOp::Owner OpsTask::OpChain::appendOp(
        GrOp::Owner op, GrProcessorSet::Analysis processorAnalysis,
        const GrDstProxyView* dstProxyView, const GrAppliedClip* appliedClip, const GrCaps& caps,
        SkArenaAlloc* opsTaskArena, GrAuditTrail* auditTrail) {
    const GrDstProxyView noDstProxyView;
    if (!dstProxyView) {
        dstProxyView = &noDstProxyView;
    }
    SkASSERT(op->isChainHead() && op->isChainTail());
    SkRect opBounds = op->bounds();
    List chain(std::move(op));
    if (!this->tryConcat(&chain, processorAnalysis, *dstProxyView, appliedClip, opBounds, caps,
                         opsTaskArena, auditTrail)) {
        // append failed, give the op back to the caller.
        this->validate();
        return chain.popHead();
    }

    SkASSERT(chain.empty());
    this->validate();
    return nullptr;
}

inline void OpsTask::OpChain::validate() const {
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

OpsTask::OpsTask(GrDrawingManager* drawingMgr,
                 GrSurfaceProxyView view,
                 GrAuditTrail* auditTrail,
                 sk_sp<GrArenas> arenas)
        : GrRenderTask()
        , fAuditTrail(auditTrail)
        , fUsesMSAASurface(view.asRenderTargetProxy()->numSamples() > 1)
        , fTargetSwizzle(view.swizzle())
        , fTargetOrigin(view.origin())
        , fArenas{std::move(arenas)}
          SkDEBUGCODE(, fNumClips(0)) {
    this->addTarget(drawingMgr, view.detachProxy());
}

void OpsTask::deleteOps() {
    for (auto& chain : fOpChains) {
        chain.deleteOps();
    }
    fOpChains.clear();
}

OpsTask::~OpsTask() {
    this->deleteOps();
}

void OpsTask::addOp(GrDrawingManager* drawingMgr, GrOp::Owner op,
                    GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
    auto addDependency = [&](GrSurfaceProxy* p, skgpu::Mipmapped mipmapped) {
        this->addDependency(drawingMgr, p, mipmapped, textureResolveManager, caps);
    };

    op->visitProxies(addDependency);

    this->recordOp(std::move(op), false/*usesMSAA*/, GrProcessorSet::EmptySetAnalysis(), nullptr,
                   nullptr, caps);
}

void OpsTask::addDrawOp(GrDrawingManager* drawingMgr, GrOp::Owner op, bool usesMSAA,
                        const GrProcessorSet::Analysis& processorAnalysis, GrAppliedClip&& clip,
                        const GrDstProxyView& dstProxyView,
                        GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
    auto addDependency = [&](GrSurfaceProxy* p, skgpu::Mipmapped mipmapped) {
        this->addSampledTexture(p);
        this->addDependency(drawingMgr, p, mipmapped, textureResolveManager, caps);
    };

    op->visitProxies(addDependency);
    clip.visitProxies(addDependency);
    if (dstProxyView.proxy()) {
        if (!(dstProxyView.dstSampleFlags() & GrDstSampleFlags::kAsInputAttachment)) {
            this->addSampledTexture(dstProxyView.proxy());
        }
        if (dstProxyView.dstSampleFlags() & GrDstSampleFlags::kRequiresTextureBarrier) {
            fRenderPassXferBarriers |= GrXferBarrierFlags::kTexture;
        }
        addDependency(dstProxyView.proxy(), skgpu::Mipmapped::kNo);
        SkASSERT(!(dstProxyView.dstSampleFlags() & GrDstSampleFlags::kAsInputAttachment) ||
                 dstProxyView.offset().isZero());
    }

    if (processorAnalysis.usesNonCoherentHWBlending()) {
        fRenderPassXferBarriers |= GrXferBarrierFlags::kBlend;
    }

    this->recordOp(std::move(op), usesMSAA, processorAnalysis, clip.doesClip() ? &clip : nullptr,
                   &dstProxyView, caps);
}

void OpsTask::endFlush(GrDrawingManager* drawingMgr) {
    fLastClipStackGenID = SK_InvalidUniqueID;
    this->deleteOps();

    fDeferredProxies.clear();
    fSampledProxies.clear();
    fAuditTrail = nullptr;

    GrRenderTask::endFlush(drawingMgr);
}

void OpsTask::onPrePrepare(GrRecordingContext* context) {
    SkASSERT(this->isClosed());
    // TODO: remove the check for discard here once reduced op splitting is turned on. Currently we
    // can end up with OpsTasks that only have a discard load op and no ops. For vulkan validation
    // we need to keep that discard and not drop it. Once we have reduce op list splitting enabled
    // we shouldn't end up with OpsTasks with only discard.
    if (this->isColorNoOp() ||
        (fClippedContentBounds.isEmpty() && fColorLoadOp != GrLoadOp::kDiscard)) {
        return;
    }
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    GrSurfaceProxyView dstView(sk_ref_sp(this->target(0)), fTargetOrigin, fTargetSwizzle);
    for (const auto& chain : fOpChains) {
        if (chain.shouldExecute()) {
            chain.head()->prePrepare(context,
                                     dstView,
                                     chain.appliedClip(),
                                     chain.dstProxyView(),
                                     fRenderPassXferBarriers,
                                     fColorLoadOp);
        }
    }
}

void OpsTask::onPrepare(GrOpFlushState* flushState) {
    SkASSERT(this->target(0)->peekRenderTarget());
    SkASSERT(this->isClosed());
    // TODO: remove the check for discard here once reduced op splitting is turned on. Currently we
    // can end up with OpsTasks that only have a discard load op and no ops. For vulkan validation
    // we need to keep that discard and not drop it. Once we have reduce op list splitting enabled
    // we shouldn't end up with OpsTasks with only discard.
    if (this->isColorNoOp() ||
        (fClippedContentBounds.isEmpty() && fColorLoadOp != GrLoadOp::kDiscard)) {
        return;
    }
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_FUNC);

    flushState->setSampledProxyArray(&fSampledProxies);
    GrSurfaceProxyView dstView(sk_ref_sp(this->target(0)), fTargetOrigin, fTargetSwizzle);
    // Loop over the ops that haven't yet been prepared.
    for (const auto& chain : fOpChains) {
        if (chain.shouldExecute()) {
            GrOpFlushState::OpArgs opArgs(chain.head(),
                                          dstView,
                                          fUsesMSAASurface,
                                          chain.appliedClip(),
                                          chain.dstProxyView(),
                                          fRenderPassXferBarriers,
                                          fColorLoadOp);

            flushState->setOpArgs(&opArgs);

            // Temporary debugging helper: for debugging prePrepare w/o going through DDLs
            // Delete once most of the GrOps have an onPrePrepare.
            // chain.head()->prePrepare(flushState->gpu()->getContext(), &this->target(0),
            //                          chain.appliedClip());

            // GrOp::prePrepare may or may not have been called at this point
            chain.head()->prepare(flushState);
            flushState->setOpArgs(nullptr);
        }
    }
    flushState->setSampledProxyArray(nullptr);
}

// TODO: this is where GrOp::renderTarget is used (which is fine since it
// is at flush time). However, we need to store the RenderTargetProxy in the
// Ops and instantiate them here.
bool OpsTask::onExecute(GrOpFlushState* flushState) {
    SkASSERT(this->numTargets() == 1);
    GrRenderTargetProxy* proxy = this->target(0)->asRenderTargetProxy();
    SkASSERT(proxy);
    SK_AT_SCOPE_EXIT(proxy->clearArenas());

    if (this->isColorNoOp() || fClippedContentBounds.isEmpty()) {
        return false;
    }
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_FUNC);

    // Make sure load ops are not kClear if the GPU needs to use draws for clears
    SkASSERT(fColorLoadOp != GrLoadOp::kClear ||
             !flushState->gpu()->caps()->performColorClearsAsDraws());

    const GrCaps& caps = *flushState->gpu()->caps();
    GrRenderTarget* renderTarget = proxy->peekRenderTarget();
    SkASSERT(renderTarget);

    GrAttachment* stencil = nullptr;
    if (proxy->needsStencil()) {
        SkASSERT(proxy->canUseStencil(caps));
        if (!flushState->resourceProvider()->attachStencilAttachment(renderTarget,
                                                                     fUsesMSAASurface)) {
            SkDebugf("WARNING: failed to attach a stencil buffer. Rendering will be skipped.\n");
            return false;
        }
        stencil = renderTarget->getStencilAttachment(fUsesMSAASurface);
    }

    GrLoadOp stencilLoadOp;
    switch (fInitialStencilContent) {
        case StencilContent::kDontCare:
            stencilLoadOp = GrLoadOp::kDiscard;
            break;
        case StencilContent::kUserBitsCleared:
            SkASSERT(!caps.performStencilClearsAsDraws());
            SkASSERT(stencil);
            if (caps.discardStencilValuesAfterRenderPass()) {
                // Always clear the stencil if it is being discarded after render passes. This is
                // also an optimization because we are on a tiler and it avoids loading the values
                // from memory.
                stencilLoadOp = GrLoadOp::kClear;
                break;
            }
            if (!stencil->hasPerformedInitialClear()) {
                stencilLoadOp = GrLoadOp::kClear;
                stencil->markHasPerformedInitialClear();
                break;
            }
            // SurfaceDrawContexts are required to leave the user stencil bits in a cleared state
            // once finished, meaning the stencil values will always remain cleared after the
            // initial clear. Just fall through to reloading the existing (cleared) stencil values
            // from memory.
            [[fallthrough]];
        case StencilContent::kPreserved:
            SkASSERT(stencil);
            stencilLoadOp = GrLoadOp::kLoad;
            break;
    }

    // NOTE: If fMustPreserveStencil is set, then we are executing a surfaceDrawContext that split
    // its opsTask.
    //
    // FIXME: We don't currently flag render passes that don't use stencil at all. In that case
    // their store op might be "discard", and we currently make the assumption that a discard will
    // not invalidate what's already in main memory. This is probably ok for now, but certainly
    // something we want to address soon.
    GrStoreOp stencilStoreOp = (caps.discardStencilValuesAfterRenderPass() && !fMustPreserveStencil)
            ? GrStoreOp::kDiscard
            : GrStoreOp::kStore;

    GrOpsRenderPass* renderPass = create_render_pass(flushState->gpu(),
                                                     proxy->peekRenderTarget(),
                                                     fUsesMSAASurface,
                                                     stencil,
                                                     fTargetOrigin,
                                                     fClippedContentBounds,
                                                     fColorLoadOp,
                                                     fLoadClearColor,
                                                     stencilLoadOp,
                                                     stencilStoreOp,
                                                     fSampledProxies,
                                                     fRenderPassXferBarriers);

    if (!renderPass) {
        return false;
    }
    flushState->setOpsRenderPass(renderPass);
    renderPass->begin();

    GrSurfaceProxyView dstView(sk_ref_sp(this->target(0)), fTargetOrigin, fTargetSwizzle);

    // Draw all the generated geometry.
    for (const auto& chain : fOpChains) {
        if (!chain.shouldExecute()) {
            continue;
        }

        GrOpFlushState::OpArgs opArgs(chain.head(),
                                      dstView,
                                      fUsesMSAASurface,
                                      chain.appliedClip(),
                                      chain.dstProxyView(),
                                      fRenderPassXferBarriers,
                                      fColorLoadOp);

        flushState->setOpArgs(&opArgs);
        chain.head()->execute(flushState, chain.bounds());
        flushState->setOpArgs(nullptr);
    }

    renderPass->end();
    flushState->gpu()->submit(renderPass);
    flushState->setOpsRenderPass(nullptr);

    return true;
}

void OpsTask::setColorLoadOp(GrLoadOp op, std::array<float, 4> color) {
    fColorLoadOp = op;
    fLoadClearColor = color;
    if (GrLoadOp::kClear == fColorLoadOp) {
        GrSurfaceProxy* proxy = this->target(0);
        SkASSERT(proxy);
        fTotalBounds = proxy->backingStoreBoundsRect();
    }
}

void OpsTask::reset() {
    fDeferredProxies.clear();
    fSampledProxies.clear();
    fClippedContentBounds = SkIRect::MakeEmpty();
    fTotalBounds = SkRect::MakeEmpty();
    this->deleteOps();
    fRenderPassXferBarriers = GrXferBarrierFlags::kNone;
}

bool OpsTask::canMerge(const OpsTask* opsTask) const {
    return this->target(0) == opsTask->target(0) &&
           fArenas == opsTask->fArenas &&
           !opsTask->fCannotMergeBackward;
}

int OpsTask::mergeFrom(SkSpan<const sk_sp<GrRenderTask>> tasks) {
    int mergedCount = 0;
    for (const sk_sp<GrRenderTask>& task : tasks) {
        auto opsTask = task->asOpsTask();
        if (!opsTask || !this->canMerge(opsTask)) {
            break;
        }
        SkASSERT(fTargetSwizzle == opsTask->fTargetSwizzle);
        SkASSERT(fTargetOrigin == opsTask->fTargetOrigin);
        if (GrLoadOp::kClear == opsTask->fColorLoadOp) {
            // TODO(11903): Go back to actually dropping ops tasks when we are merged with
            // color clear.
            return 0;
        }
        mergedCount += 1;
    }
    if (0 == mergedCount) {
        return 0;
    }

    SkSpan<const sk_sp<OpsTask>> mergingNodes(
            reinterpret_cast<const sk_sp<OpsTask>*>(tasks.data()), SkToSizeT(mergedCount));
    int addlDeferredProxyCount = 0;
    int addlProxyCount = 0;
    int addlOpChainCount = 0;
    for (const auto& toMerge : mergingNodes) {
        addlDeferredProxyCount += toMerge->fDeferredProxies.size();
        addlProxyCount += toMerge->fSampledProxies.size();
        addlOpChainCount += toMerge->fOpChains.size();
        fClippedContentBounds.join(toMerge->fClippedContentBounds);
        fTotalBounds.join(toMerge->fTotalBounds);
        fRenderPassXferBarriers |= toMerge->fRenderPassXferBarriers;
        if (fInitialStencilContent == StencilContent::kDontCare) {
            // Propogate the first stencil content that isn't kDontCare.
            //
            // Once the stencil has any kind of initial content that isn't kDontCare, then the
            // inital contents of subsequent opsTasks that get merged in don't matter.
            //
            // (This works because the opsTask all target the same render target and are in
            // painter's order. kPreserved obviously happens automatically with a merge, and kClear
            // is also automatic because the contract is for ops to leave the stencil buffer in a
            // cleared state when finished.)
            fInitialStencilContent = toMerge->fInitialStencilContent;
        }
        fUsesMSAASurface |= toMerge->fUsesMSAASurface;
        SkDEBUGCODE(fNumClips += toMerge->fNumClips);
    }

    fLastClipStackGenID = SK_InvalidUniqueID;
    fDeferredProxies.reserve_exact(fDeferredProxies.size() + addlDeferredProxyCount);
    fSampledProxies.reserve_exact(fSampledProxies.size() + addlProxyCount);
    fOpChains.reserve_exact(fOpChains.size() + addlOpChainCount);
    for (const auto& toMerge : mergingNodes) {
        for (GrRenderTask* renderTask : toMerge->dependents()) {
            renderTask->replaceDependency(toMerge.get(), this);
        }
        for (GrRenderTask* renderTask : toMerge->dependencies()) {
            renderTask->replaceDependent(toMerge.get(), this);
        }
        fDeferredProxies.move_back_n(toMerge->fDeferredProxies.size(),
                                     toMerge->fDeferredProxies.data());
        fSampledProxies.move_back_n(toMerge->fSampledProxies.size(),
                                    toMerge->fSampledProxies.data());
        fOpChains.move_back_n(toMerge->fOpChains.size(),
                              toMerge->fOpChains.data());
        toMerge->fDeferredProxies.clear();
        toMerge->fSampledProxies.clear();
        toMerge->fOpChains.clear();
    }
    fMustPreserveStencil = mergingNodes.back()->fMustPreserveStencil;
    return mergedCount;
}

bool OpsTask::resetForFullscreenClear(CanDiscardPreviousOps canDiscardPreviousOps) {
    if (CanDiscardPreviousOps::kYes == canDiscardPreviousOps || this->isEmpty()) {
        this->deleteOps();
        fDeferredProxies.clear();
        fSampledProxies.clear();

        // If the opsTask is using a render target which wraps a vulkan command buffer, we can't do
        // a clear load since we cannot change the render pass that we are using. Thus we fall back
        // to making a clear op in this case.
        return !this->target(0)->asRenderTargetProxy()->wrapsVkSecondaryCB();
    }

    // Could not empty the task, so an op must be added to handle the clear
    return false;
}

void OpsTask::discard() {
    // Discard calls to in-progress opsTasks are ignored. Calls at the start update the
    // opsTasks' color & stencil load ops.
    if (this->isEmpty()) {
        fColorLoadOp = GrLoadOp::kDiscard;
        fInitialStencilContent = StencilContent::kDontCare;
        fTotalBounds.setEmpty();
    }
}

////////////////////////////////////////////////////////////////////////////////

#if defined(GR_TEST_UTILS)
void OpsTask::dump(const SkString& label,
                   SkString indent,
                   bool printDependencies,
                   bool close) const {
    GrRenderTask::dump(label, indent, printDependencies, false);

    SkDebugf("%sfColorLoadOp: ", indent.c_str());
    switch (fColorLoadOp) {
        case GrLoadOp::kLoad:
            SkDebugf("kLoad\n");
            break;
        case GrLoadOp::kClear:
            SkDebugf("kClear {%g, %g, %g, %g}\n",
                     fLoadClearColor[0],
                     fLoadClearColor[1],
                     fLoadClearColor[2],
                     fLoadClearColor[3]);
            break;
        case GrLoadOp::kDiscard:
            SkDebugf("kDiscard\n");
            break;
    }

    SkDebugf("%sfInitialStencilContent: ", indent.c_str());
    switch (fInitialStencilContent) {
        case StencilContent::kDontCare:
            SkDebugf("kDontCare\n");
            break;
        case StencilContent::kUserBitsCleared:
            SkDebugf("kUserBitsCleared\n");
            break;
        case StencilContent::kPreserved:
            SkDebugf("kPreserved\n");
            break;
    }

    SkDebugf("%s%d ops:\n", indent.c_str(), fOpChains.size());
    for (int i = 0; i < fOpChains.size(); ++i) {
        SkDebugf("%s*******************************\n", indent.c_str());
        if (!fOpChains[i].head()) {
            SkDebugf("%s%d: <combined forward or failed instantiation>\n", indent.c_str(), i);
        } else {
            SkDebugf("%s%d: %s\n", indent.c_str(), i, fOpChains[i].head()->name());
            SkRect bounds = fOpChains[i].bounds();
            SkDebugf("%sClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                     indent.c_str(),
                     bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
            for (const auto& op : GrOp::ChainRange<>(fOpChains[i].head())) {
                SkString info = SkTabString(op.dumpInfo(), 1);
                SkDebugf("%s%s\n", indent.c_str(), info.c_str());
                bounds = op.bounds();
                SkDebugf("%s\tClippedBounds: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                         indent.c_str(),
                         bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
            }
        }
    }

    if (close) {
        SkDebugf("%s--------------------------------------------------------------\n\n",
                 indent.c_str());
    }
}
#endif

#ifdef SK_DEBUG
void OpsTask::visitProxies_debugOnly(const GrVisitProxyFunc& func) const {
    auto textureFunc = [func](GrSurfaceProxy* tex, skgpu::Mipmapped mipmapped) {
        func(tex, mipmapped);
    };

    for (const OpChain& chain : fOpChains) {
        chain.visitProxies(textureFunc);
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////

void OpsTask::onMakeSkippable() {
    this->deleteOps();
    fDeferredProxies.clear();
    fColorLoadOp = GrLoadOp::kLoad;
    SkASSERT(this->isColorNoOp());
}

bool OpsTask::onIsUsed(GrSurfaceProxy* proxyToCheck) const {
    bool used = false;
    for (GrSurfaceProxy* proxy : fSampledProxies) {
        if (proxy == proxyToCheck) {
            used = true;
            break;
        }
    }
#ifdef SK_DEBUG
    bool usedSlow = false;
    auto visit = [proxyToCheck, &usedSlow](GrSurfaceProxy* p, skgpu::Mipmapped) {
        if (p == proxyToCheck) {
            usedSlow = true;
        }
    };
    this->visitProxies_debugOnly(visit);
    SkASSERT(used == usedSlow);
#endif

    return used;
}

void OpsTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    SkASSERT(this->isClosed());
    if (this->isColorNoOp()) {
        return;
    }

    for (int i = 0; i < fDeferredProxies.size(); ++i) {
        SkASSERT(!fDeferredProxies[i]->isInstantiated());
        // We give all the deferred proxies a write usage at the very start of flushing. This
        // locks them out of being reused for the entire flush until they are read - and then
        // they can be recycled. This is a bit unfortunate because a flush can proceed in waves
        // with sub-flushes. The deferred proxies only need to be pinned from the start of
        // the sub-flush in which they appear.
        alloc->addInterval(fDeferredProxies[i], 0, 0, GrResourceAllocator::ActualUse::kNo,
                           GrResourceAllocator::AllowRecycling::kYes);
    }

    GrSurfaceProxy* targetSurface = this->target(0);
    SkASSERT(targetSurface);
    GrRenderTargetProxy* targetProxy = targetSurface->asRenderTargetProxy();

    // Add the interval for all the writes to this OpsTasks's target
    if (fOpChains.size()) {
        unsigned int cur = alloc->curOp();

        alloc->addInterval(targetProxy, cur, cur + fOpChains.size() - 1,
                           GrResourceAllocator::ActualUse::kYes,
                           GrResourceAllocator::AllowRecycling::kYes);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(targetProxy, alloc->curOp(), alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes,
                           GrResourceAllocator::AllowRecycling::kYes);
        alloc->incOps();
    }

    GrResourceAllocator::AllowRecycling allowRecycling =
            targetProxy->wrapsVkSecondaryCB() ? GrResourceAllocator::AllowRecycling::kNo
                                              : GrResourceAllocator::AllowRecycling::kYes;

    auto gather = [alloc, allowRecycling SkDEBUGCODE(, this)](GrSurfaceProxy* p, skgpu::Mipmapped) {
        alloc->addInterval(p,
                           alloc->curOp(),
                           alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes,
                           allowRecycling
                           SkDEBUGCODE(, this->target(0) == p));
    };
    // TODO: visitProxies is expensive. Can we do this with fSampledProxies instead?
    for (const OpChain& recordedOp : fOpChains) {
        recordedOp.visitProxies(gather);

        // Even though the op may have been (re)moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

void OpsTask::recordOp(
        GrOp::Owner op, bool usesMSAA, GrProcessorSet::Analysis processorAnalysis,
        GrAppliedClip* clip, const GrDstProxyView* dstProxyView, const GrCaps& caps) {
    GrSurfaceProxy* proxy = this->target(0);
#ifdef SK_DEBUG
    op->validate();
    SkASSERT(processorAnalysis.requiresDstTexture() == (dstProxyView && dstProxyView->proxy()));
    SkASSERT(proxy);
    // A closed OpsTask should never receive new/more ops
    SkASSERT(!this->isClosed());
    // Ensure we can support dynamic msaa if the caller is trying to trigger it.
    if (proxy->asRenderTargetProxy()->numSamples() == 1 && usesMSAA) {
        SkASSERT(caps.supportsDynamicMSAA(proxy->asRenderTargetProxy()));
    }
#endif

    if (!op->bounds().isFinite()) {
        return;
    }

    fUsesMSAASurface |= usesMSAA;

    // Account for this op's bounds before we attempt to combine.
    // NOTE: The caller should have already called "op->setClippedBounds()" by now, if applicable.
    fTotalBounds.join(op->bounds());

    // Check if there is an op we can combine with by linearly searching back until we either
    // 1) check every op
    // 2) intersect with something
    // 3) find a 'blocker'
    GR_AUDIT_TRAIL_ADD_OP(fAuditTrail, op.get(), proxy->uniqueID());
    GrOP_INFO("opsTask: %d Recording (%s, opID: %u)\n"
              "\tBounds [L: %.2f, T: %.2f R: %.2f B: %.2f]\n",
               this->uniqueID(),
               op->name(),
               op->uniqueID(),
               op->bounds().fLeft, op->bounds().fTop,
               op->bounds().fRight, op->bounds().fBottom);
    GrOP_INFO(SkTabString(op->dumpInfo(), 1).c_str());
    GrOP_INFO("\tOutcome:\n");
    int maxCandidates = std::min(kMaxOpChainDistance, fOpChains.size());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            OpChain& candidate = fOpChains.fromBack(i);
            op = candidate.appendOp(std::move(op), processorAnalysis, dstProxyView, clip, caps,
                                    fArenas->arenaAlloc(), fAuditTrail);
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
        clip = fArenas->arenaAlloc()->make<GrAppliedClip>(std::move(*clip));
        SkDEBUGCODE(fNumClips++;)
    }
    fOpChains.emplace_back(std::move(op), processorAnalysis, clip, dstProxyView);
}

void OpsTask::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());
    GrOP_INFO("opsTask: %d ForwardCombine %d ops:\n", this->uniqueID(), fOpChains.size());

    for (int i = 0; i < fOpChains.size() - 1; ++i) {
        OpChain& chain = fOpChains[i];
        int maxCandidateIdx = std::min(i + kMaxOpChainDistance, fOpChains.size() - 1);
        int j = i + 1;
        while (true) {
            OpChain& candidate = fOpChains[j];
            if (candidate.prependChain(&chain, caps, fArenas->arenaAlloc(), fAuditTrail)) {
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

GrRenderTask::ExpectedOutcome OpsTask::onMakeClosed(GrRecordingContext* rContext,
                                                    SkIRect* targetUpdateBounds) {
    this->forwardCombine(*rContext->priv().caps());
    if (!this->isColorNoOp()) {
        GrSurfaceProxy* proxy = this->target(0);
        // Use the entire backing store bounds since the GPU doesn't clip automatically to the
        // logical dimensions.
        SkRect clippedContentBounds = proxy->backingStoreBoundsRect();
        // TODO: If we can fix up GLPrograms test to always intersect the target proxy bounds
        // then we can simply assert here that the bounds intersect.
        if (clippedContentBounds.intersect(fTotalBounds)) {
            clippedContentBounds.roundOut(&fClippedContentBounds);
            *targetUpdateBounds = GrNativeRect::MakeIRectRelativeTo(
                    fTargetOrigin,
                    this->target(0)->backingStoreDimensions().height(),
                    fClippedContentBounds);
            return ExpectedOutcome::kTargetDirty;
        }
    }
    return ExpectedOutcome::kTargetUnchanged;
}

}  // namespace skgpu::ganesh
