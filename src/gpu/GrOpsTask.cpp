/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrOpsTask.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/ops/GrClearOp.h"

////////////////////////////////////////////////////////////////////////////////

// Experimentally we have found that most combining occurs within the first 10 comparisons.
static const int kMaxOpMergeDistance = 10;
static const int kMaxOpChainDistance = 10;

////////////////////////////////////////////////////////////////////////////////

using DstProxyView = GrXferProcessor::DstProxyView;

////////////////////////////////////////////////////////////////////////////////

static inline bool can_reorder(const SkRect& a, const SkRect& b) { return !GrRectsOverlap(a, b); }

////////////////////////////////////////////////////////////////////////////////

inline GrOpsTask::OpChain::List::List(GrOp::Owner op)
        : fHead(std::move(op)), fTail(fHead.get()) {
    this->validate();
}

inline GrOpsTask::OpChain::List::List(List&& that) { *this = std::move(that); }

inline GrOpsTask::OpChain::List& GrOpsTask::OpChain::List::operator=(List&& that) {
    fHead = std::move(that.fHead);
    fTail = that.fTail;
    that.fTail = nullptr;
    this->validate();
    return *this;
}

inline GrOp::Owner GrOpsTask::OpChain::List::popHead() {
    SkASSERT(fHead);
    auto temp = fHead->cutChain();
    std::swap(temp, fHead);
    if (!fHead) {
        SkASSERT(fTail == temp.get());
        fTail = nullptr;
    }
    return temp;
}

inline GrOp::Owner GrOpsTask::OpChain::List::removeOp(GrOp* op) {
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

inline void GrOpsTask::OpChain::List::pushHead(GrOp::Owner op) {
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

inline void GrOpsTask::OpChain::List::pushTail(GrOp::Owner op) {
    SkASSERT(op->isChainTail());
    fTail->chainConcat(std::move(op));
    fTail = fTail->nextInChain();
}

inline void GrOpsTask::OpChain::List::validate() const {
#ifdef SK_DEBUG
    if (fHead) {
        SkASSERT(fTail);
        fHead->validateChain(fTail);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

GrOpsTask::OpChain::OpChain(GrOp::Owner op,
                            GrProcessorSet::Analysis processorAnalysis,
                            GrAppliedClip* appliedClip, const DstProxyView* dstProxyView)
        : fList{std::move(op)}
        , fProcessorAnalysis(processorAnalysis)
        , fAppliedClip(appliedClip) {
    if (fProcessorAnalysis.requiresDstTexture()) {
        SkASSERT(dstProxyView && dstProxyView->proxy());
        fDstProxyView = *dstProxyView;
    }
    fBounds = fList.head()->bounds();
}

void GrOpsTask::OpChain::visitProxies(const GrOp::VisitProxyFunc& func) const {
    if (fList.empty()) {
        return;
    }
    for (const auto& op : GrOp::ChainRange<>(fList.head())) {
        op.visitProxies(func);
    }
    if (fDstProxyView.proxy()) {
        func(fDstProxyView.proxy(), GrMipmapped::kNo);
    }
    if (fAppliedClip) {
        fAppliedClip->visitProxies(func);
    }
}

void GrOpsTask::OpChain::deleteOps() {
    while (!fList.empty()) {
        // Since the value goes out of scope immediately, the GrOp::Owner deletes the op.
        fList.popHead();
    }
}

// Concatenates two op chains and attempts to merge ops across the chains. Assumes that we know that
// the two chains are chainable. Returns the new chain.
GrOpsTask::OpChain::List GrOpsTask::OpChain::DoConcat(
        List chainA, List chainB, const GrCaps& caps, GrRecordingContext::Arenas* arenas,
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
                auto result = a->combineIfPossible(
                        chainB.head(), arenas->recordTimeAllocator(), caps);
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
bool GrOpsTask::OpChain::tryConcat(
        List* list, GrProcessorSet::Analysis processorAnalysis, const DstProxyView& dstProxyView,
        const GrAppliedClip* appliedClip, const SkRect& bounds, const GrCaps& caps,
        GrRecordingContext::Arenas* arenas, GrAuditTrail* auditTrail) {
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
        switch (fList.tail()->combineIfPossible(list->head(), arenas->recordTimeAllocator(), caps))
        {
            case GrOp::CombineResult::kCannotCombine:
                // If an op supports chaining then it is required that chaining is transitive and
                // that if any two ops in two different chains can merge then the two chains
                // may also be chained together. Thus, we should only hit this on the first
                // iteration.
                SkASSERT(first);
                return false;
            case GrOp::CombineResult::kMayChain:
                fList = DoConcat(std::move(fList), std::exchange(*list, List()), caps, arenas,
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

bool GrOpsTask::OpChain::prependChain(OpChain* that, const GrCaps& caps,
                                      GrRecordingContext::Arenas* arenas,
                                      GrAuditTrail* auditTrail) {
    if (!that->tryConcat(&fList, fProcessorAnalysis, fDstProxyView, fAppliedClip, fBounds, caps,
                         arenas, auditTrail)) {
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

GrOp::Owner GrOpsTask::OpChain::appendOp(
        GrOp::Owner op, GrProcessorSet::Analysis processorAnalysis,
        const DstProxyView* dstProxyView, const GrAppliedClip* appliedClip, const GrCaps& caps,
        GrRecordingContext::Arenas* arenas, GrAuditTrail* auditTrail) {
    const GrXferProcessor::DstProxyView noDstProxyView;
    if (!dstProxyView) {
        dstProxyView = &noDstProxyView;
    }
    SkASSERT(op->isChainHead() && op->isChainTail());
    SkRect opBounds = op->bounds();
    List chain(std::move(op));
    if (!this->tryConcat(
            &chain, processorAnalysis, *dstProxyView, appliedClip, opBounds, caps,
            arenas, auditTrail)) {
        // append failed, give the op back to the caller.
        this->validate();
        return chain.popHead();
    }

    SkASSERT(chain.empty());
    this->validate();
    return nullptr;
}

inline void GrOpsTask::OpChain::validate() const {
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

GrOpsTask::GrOpsTask(GrDrawingManager* drawingMgr,
                     GrRecordingContext::Arenas arenas,
                     GrSurfaceProxyView view,
                     GrAuditTrail* auditTrail)
        : GrRenderTask()
        , fArenas(arenas)
        , fAuditTrail(auditTrail)
        , fTargetSwizzle(view.swizzle())
        , fTargetOrigin(view.origin())
          SkDEBUGCODE(, fNumClips(0)) {
    fClipAllocators.push_back(std::make_unique<SkArenaAlloc>(4096));
    this->addTarget(drawingMgr, view.detachProxy());
}

void GrOpsTask::deleteOps() {
    for (auto& chain : fOpChains) {
        chain.deleteOps();
    }
    fOpChains.reset();
}

GrOpsTask::~GrOpsTask() {
    this->deleteOps();
}

void GrOpsTask::addOp(GrDrawingManager* drawingMgr, GrOp::Owner op,
                      GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
    auto addDependency = [&](GrSurfaceProxy* p, GrMipmapped mipmapped) {
        this->addDependency(drawingMgr, p, mipmapped, textureResolveManager, caps);
    };

    op->visitProxies(addDependency);

    this->recordOp(std::move(op), GrProcessorSet::EmptySetAnalysis(), nullptr, nullptr, caps);
}

void GrOpsTask::addDrawOp(GrDrawingManager* drawingMgr, GrOp::Owner op,
                          const GrProcessorSet::Analysis& processorAnalysis,
                          GrAppliedClip&& clip, const DstProxyView& dstProxyView,
                          GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
    auto addDependency = [&](GrSurfaceProxy* p, GrMipmapped mipmapped) {
        this->addSampledTexture(p);
        this->addDependency(drawingMgr, p, mipmapped, textureResolveManager, caps);
    };

    op->visitProxies(addDependency);
    clip.visitProxies(addDependency);
    if (dstProxyView.proxy()) {
        if (GrDstSampleTypeUsesTexture(dstProxyView.dstSampleType())) {
            this->addSampledTexture(dstProxyView.proxy());
        }
        addDependency(dstProxyView.proxy(), GrMipmapped::kNo);
        if (this->target(0) == dstProxyView.proxy()) {
            // Since we are sampling and drawing to the same surface we will need to use
            // texture barriers.
            SkASSERT(GrDstSampleTypeDirectlySamplesDst(dstProxyView.dstSampleType()));
            fRenderPassXferBarriers |= GrXferBarrierFlags::kTexture;
        }
        SkASSERT(dstProxyView.dstSampleType() != GrDstSampleType::kAsInputAttachment ||
                 dstProxyView.offset().isZero());
    }

    if (processorAnalysis.usesNonCoherentHWBlending()) {
        fRenderPassXferBarriers |= GrXferBarrierFlags::kBlend;
    }

    this->recordOp(std::move(op), processorAnalysis, clip.doesClip() ? &clip : nullptr,
                   &dstProxyView, caps);
}

void GrOpsTask::endFlush(GrDrawingManager* drawingMgr) {
    fLastClipStackGenID = SK_InvalidUniqueID;
    this->deleteOps();
    fClipAllocators.reset();

    fDeferredProxies.reset();
    fSampledProxies.reset();
    fAuditTrail = nullptr;

    GrRenderTask::endFlush(drawingMgr);
}

void GrOpsTask::onPrePrepare(GrRecordingContext* context) {
    SkASSERT(this->isClosed());
    // TODO: remove the check for discard here once reduced op splitting is turned on. Currently we
    // can end up with GrOpsTasks that only have a discard load op and no ops. For vulkan validation
    // we need to keep that discard and not drop it. Once we have reduce op list splitting enabled
    // we shouldn't end up with GrOpsTasks with only discard.
    if (this->isNoOp() || (fClippedContentBounds.isEmpty() && fColorLoadOp != GrLoadOp::kDiscard)) {
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

void GrOpsTask::onPrepare(GrOpFlushState* flushState) {
    SkASSERT(this->target(0)->peekRenderTarget());
    SkASSERT(this->isClosed());
    // TODO: remove the check for discard here once reduced op splitting is turned on. Currently we
    // can end up with GrOpsTasks that only have a discard load op and no ops. For vulkan validation
    // we need to keep that discard and not drop it. Once we have reduce op list splitting enabled
    // we shouldn't end up with GrOpsTasks with only discard.
    if (this->isNoOp() || (fClippedContentBounds.isEmpty() && fColorLoadOp != GrLoadOp::kDiscard)) {
        return;
    }
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    flushState->setSampledProxyArray(&fSampledProxies);
    GrSurfaceProxyView dstView(sk_ref_sp(this->target(0)), fTargetOrigin, fTargetSwizzle);
    // Loop over the ops that haven't yet been prepared.
    for (const auto& chain : fOpChains) {
        if (chain.shouldExecute()) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
            TRACE_EVENT0("skia.gpu", chain.head()->name());
#endif
            GrOpFlushState::OpArgs opArgs(chain.head(),
                                          dstView,
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

static GrOpsRenderPass* create_render_pass(GrGpu* gpu,
                                           GrRenderTarget* rt,
                                           GrAttachment* stencil,
                                           GrSurfaceOrigin origin,
                                           const SkIRect& bounds,
                                           GrLoadOp colorLoadOp,
                                           const std::array<float, 4>& loadClearColor,
                                           GrLoadOp stencilLoadOp,
                                           GrStoreOp stencilStoreOp,
                                           const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
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

    return gpu->getOpsRenderPass(rt, stencil, origin, bounds,
                                 kColorLoadStoreInfo, stencilLoadAndStoreInfo, sampledProxies,
                                 renderPassXferBarriers);
}

// TODO: this is where GrOp::renderTarget is used (which is fine since it
// is at flush time). However, we need to store the RenderTargetProxy in the
// Ops and instantiate them here.
bool GrOpsTask::onExecute(GrOpFlushState* flushState) {
    // TODO: remove the check for discard here once reduced op splitting is turned on. Currently we
    // can end up with GrOpsTasks that only have a discard load op and no ops. For vulkan validation
    // we need to keep that discard and not drop it. Once we have reduce op list splitting enabled
    // we shouldn't end up with GrOpsTasks with only discard.
    if (this->isNoOp() || (fClippedContentBounds.isEmpty() && fColorLoadOp != GrLoadOp::kDiscard)) {
        return false;
    }

    SkASSERT(this->numTargets() == 1);
    GrRenderTargetProxy* proxy = this->target(0)->asRenderTargetProxy();
    SkASSERT(proxy);
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // Make sure load ops are not kClear if the GPU needs to use draws for clears
    SkASSERT(fColorLoadOp != GrLoadOp::kClear ||
             !flushState->gpu()->caps()->performColorClearsAsDraws());

    const GrCaps& caps = *flushState->gpu()->caps();
    GrRenderTarget* renderTarget = proxy->peekRenderTarget();
    SkASSERT(renderTarget);

    GrAttachment* stencil = nullptr;
    if (int numStencilSamples = proxy->numStencilSamples()) {
        if (!flushState->resourceProvider()->attachStencilAttachment(
                renderTarget, numStencilSamples)) {
            SkDebugf("WARNING: failed to attach a stencil buffer. Rendering will be skipped.\n");
            return false;
        }
        stencil = renderTarget->getStencilAttachment();
    }

    SkASSERT(!stencil || stencil->numSamples() == proxy->numStencilSamples());

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
            // renderTargetContexts are required to leave the user stencil bits in a cleared state
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
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        TRACE_EVENT0("skia.gpu", chain.head()->name());
#endif

        GrOpFlushState::OpArgs opArgs(chain.head(),
                                      dstView,
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

void GrOpsTask::setColorLoadOp(GrLoadOp op, std::array<float, 4> color) {
    fColorLoadOp = op;
    fLoadClearColor = color;
    if (GrLoadOp::kClear == fColorLoadOp) {
        GrSurfaceProxy* proxy = this->target(0);
        SkASSERT(proxy);
        fTotalBounds = proxy->backingStoreBoundsRect();
    }
}

void GrOpsTask::reset() {
    fDeferredProxies.reset();
    fSampledProxies.reset();
    fClipAllocators.reset();
    fClippedContentBounds = SkIRect::MakeEmpty();
    fTotalBounds = SkRect::MakeEmpty();
    fOpChains.reset();
    fRenderPassXferBarriers = GrXferBarrierFlags::kNone;
}

int GrOpsTask::mergeFrom(SkSpan<const sk_sp<GrRenderTask>> tasks) {
    // Find the index of the last color-clearing task. -1 indicates this or "there are none."
    int indexOfLastColorClear = -1;
    int mergedCount = 0;
    for (const sk_sp<GrRenderTask>& task : tasks) {
        auto opsTask = task->asOpsTask();
        if (!opsTask || opsTask->target(0) != this->target(0)) {
            break;
        }
        SkASSERT(fTargetSwizzle == opsTask->fTargetSwizzle);
        SkASSERT(fTargetOrigin == opsTask->fTargetOrigin);
        if (GrLoadOp::kClear == opsTask->fColorLoadOp) {
            indexOfLastColorClear = &task - tasks.begin();
        }
        mergedCount += 1;
    }
    if (0 == mergedCount) {
        return 0;
    }

    SkSpan<const sk_sp<GrOpsTask>> opsTasks(reinterpret_cast<const sk_sp<GrOpsTask>*>(tasks.data()),
                                            SkToSizeT(mergedCount));
    if (indexOfLastColorClear >= 0) {
        // If any dropped task needs to preserve stencil, for now just bail on the merge.
        // Could keep the merge and insert a clear op, but might be tricky due to closed task.
        if (fMustPreserveStencil) {
            return 0;
        }
        for (const auto& opsTask : opsTasks.first(indexOfLastColorClear)) {
            if (opsTask->fMustPreserveStencil) {
                return 0;
            }
        }
        // Clear `this` and forget about the tasks pre-color-clear.
        this->reset();
        opsTasks = opsTasks.last(opsTasks.count() - indexOfLastColorClear);
        // Copy the color-clear into `this`.
        fColorLoadOp = GrLoadOp::kClear;
        fLoadClearColor = opsTasks.front()->fLoadClearColor;
    }
    int addlDeferredProxyCount = 0;
    int addlProxyCount = 0;
    int addlOpChainCount = 0;
    for (const auto& opsTask : opsTasks) {
        addlDeferredProxyCount += opsTask->fDeferredProxies.count();
        addlProxyCount += opsTask->fSampledProxies.count();
        addlOpChainCount += opsTask->fOpChains.count();
        fClippedContentBounds.join(opsTask->fClippedContentBounds);
        fTotalBounds.join(opsTask->fTotalBounds);
        fRenderPassXferBarriers |= opsTask->fRenderPassXferBarriers;
        SkDEBUGCODE(fNumClips += opsTask->fNumClips);
    }

    fLastClipStackGenID = SK_InvalidUniqueID;
    fDeferredProxies.reserve_back(addlDeferredProxyCount);
    fSampledProxies.reserve_back(addlProxyCount);
    fOpChains.reserve_back(addlOpChainCount);
    fClipAllocators.reserve_back(opsTasks.count());
    for (const auto& opsTask : opsTasks) {
        fDeferredProxies.move_back_n(opsTask->fDeferredProxies.count(),
                                     opsTask->fDeferredProxies.data());
        fSampledProxies.move_back_n(opsTask->fSampledProxies.count(),
                                    opsTask->fSampledProxies.data());
        fOpChains.move_back_n(opsTask->fOpChains.count(),
                              opsTask->fOpChains.data());
        SkASSERT(1 == opsTask->fClipAllocators.count());
        fClipAllocators.push_back(std::move(opsTask->fClipAllocators[0]));
        opsTask->fClipAllocators.reset();
        opsTask->fDeferredProxies.reset();
        opsTask->fSampledProxies.reset();
        opsTask->fOpChains.reset();
    }
    fMustPreserveStencil = opsTasks.back()->fMustPreserveStencil;
    return mergedCount;
}

bool GrOpsTask::resetForFullscreenClear(CanDiscardPreviousOps canDiscardPreviousOps) {
    if (CanDiscardPreviousOps::kYes == canDiscardPreviousOps || this->isEmpty()) {
        this->deleteOps();
        fDeferredProxies.reset();
        fSampledProxies.reset();

        // If the opsTask is using a render target which wraps a vulkan command buffer, we can't do
        // a clear load since we cannot change the render pass that we are using. Thus we fall back
        // to making a clear op in this case.
        return !this->target(0)->asRenderTargetProxy()->wrapsVkSecondaryCB();
    }

    // Could not empty the task, so an op must be added to handle the clear
    return false;
}

void GrOpsTask::discard() {
    // Discard calls to in-progress opsTasks are ignored. Calls at the start update the
    // opsTasks' color & stencil load ops.
    if (this->isEmpty()) {
        fColorLoadOp = GrLoadOp::kDiscard;
        fInitialStencilContent = StencilContent::kDontCare;
        fTotalBounds.setEmpty();
    }
}

////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
void GrOpsTask::dump(const SkString& label,
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

    SkDebugf("%s%d ops:\n", indent.c_str(), fOpChains.count());
    for (int i = 0; i < fOpChains.count(); ++i) {
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
void GrOpsTask::visitProxies_debugOnly(const GrOp::VisitProxyFunc& func) const {
    auto textureFunc = [ func ] (GrSurfaceProxy* tex, GrMipmapped mipmapped) {
        func(tex, mipmapped);
    };

    for (const OpChain& chain : fOpChains) {
        chain.visitProxies(textureFunc);
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////

void GrOpsTask::onCanSkip() {
    this->deleteOps();
    fDeferredProxies.reset();
    fColorLoadOp = GrLoadOp::kLoad;
    SkASSERT(this->isNoOp());
}

bool GrOpsTask::onIsUsed(GrSurfaceProxy* proxyToCheck) const {
    bool used = false;

    auto visit = [ proxyToCheck, &used ] (GrSurfaceProxy* p, GrMipmapped) {
        if (p == proxyToCheck) {
            used = true;
        }
    };
    for (const OpChain& recordedOp : fOpChains) {
        recordedOp.visitProxies(visit);
    }

    return used;
}

void GrOpsTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        SkASSERT(!fDeferredProxies[i]->isInstantiated());
        // We give all the deferred proxies a write usage at the very start of flushing. This
        // locks them out of being reused for the entire flush until they are read - and then
        // they can be recycled. This is a bit unfortunate because a flush can proceed in waves
        // with sub-flushes. The deferred proxies only need to be pinned from the start of
        // the sub-flush in which they appear.
        alloc->addInterval(fDeferredProxies[i], 0, 0, GrResourceAllocator::ActualUse::kNo);
    }

    GrSurfaceProxy* targetProxy = this->target(0);

    // Add the interval for all the writes to this GrOpsTasks's target
    if (fOpChains.count()) {
        unsigned int cur = alloc->curOp();

        alloc->addInterval(targetProxy, cur, cur + fOpChains.count() - 1,
                           GrResourceAllocator::ActualUse::kYes);
    } else {
        // This can happen if there is a loadOp (e.g., a clear) but no other draws. In this case we
        // still need to add an interval for the destination so we create a fake op# for
        // the missing clear op.
        alloc->addInterval(targetProxy, alloc->curOp(), alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes);
        alloc->incOps();
    }

    auto gather = [ alloc SkDEBUGCODE(, this) ] (GrSurfaceProxy* p, GrMipmapped) {
        alloc->addInterval(p,
                           alloc->curOp(),
                           alloc->curOp(),
                           GrResourceAllocator::ActualUse::kYes
                           SkDEBUGCODE(, this->target(0) == p));
    };
    for (const OpChain& recordedOp : fOpChains) {
        recordedOp.visitProxies(gather);

        // Even though the op may have been (re)moved we still need to increment the op count to
        // keep all the math consistent.
        alloc->incOps();
    }
}

void GrOpsTask::recordOp(
        GrOp::Owner op, GrProcessorSet::Analysis processorAnalysis, GrAppliedClip* clip,
        const DstProxyView* dstProxyView, const GrCaps& caps) {
    SkDEBUGCODE(op->validate();)
    SkASSERT(processorAnalysis.requiresDstTexture() == (dstProxyView && dstProxyView->proxy()));
    GrSurfaceProxy* proxy = this->target(0);
    SkASSERT(proxy);

    // A closed GrOpsTask should never receive new/more ops
    SkASSERT(!this->isClosed());
    if (!op->bounds().isFinite()) {
        return;
    }

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
    int maxCandidates = std::min(kMaxOpChainDistance, fOpChains.count());
    if (maxCandidates) {
        int i = 0;
        while (true) {
            OpChain& candidate = fOpChains.fromBack(i);
            op = candidate.appendOp(std::move(op), processorAnalysis, dstProxyView, clip, caps,
                                    &fArenas, fAuditTrail);
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
        clip = fClipAllocators[0]->make<GrAppliedClip>(std::move(*clip));
        SkDEBUGCODE(fNumClips++;)
    }
    fOpChains.emplace_back(std::move(op), processorAnalysis, clip, dstProxyView);
}

void GrOpsTask::forwardCombine(const GrCaps& caps) {
    SkASSERT(!this->isClosed());
    GrOP_INFO("opsTask: %d ForwardCombine %d ops:\n", this->uniqueID(), fOpChains.count());

    for (int i = 0; i < fOpChains.count() - 1; ++i) {
        OpChain& chain = fOpChains[i];
        int maxCandidateIdx = std::min(i + kMaxOpChainDistance, fOpChains.count() - 1);
        int j = i + 1;
        while (true) {
            OpChain& candidate = fOpChains[j];
            if (candidate.prependChain(&chain, caps, &fArenas, fAuditTrail)) {
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

GrRenderTask::ExpectedOutcome GrOpsTask::onMakeClosed(const GrCaps& caps,
                                                      SkIRect* targetUpdateBounds) {
    this->forwardCombine(caps);
    if (!this->isNoOp()) {
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
