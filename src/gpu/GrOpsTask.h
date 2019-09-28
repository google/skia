/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpsTask_DEFINED
#define GrOpsTask_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"

class GrAuditTrail;
class GrCaps;
class GrClearOp;
class GrGpuBuffer;
class GrOpMemoryPool;
class GrRenderTargetProxy;

class GrOpsTask : public GrRenderTask {
private:
    using DstProxy = GrXferProcessor::DstProxy;

public:
    GrOpsTask(sk_sp<GrOpMemoryPool>, sk_sp<GrRenderTargetProxy>, GrAuditTrail*);
    ~GrOpsTask() override;

    GrOpsTask* asOpsTask() override { return this; }

    bool isEmpty() const { return fOpChains.empty(); }

    /**
     * Empties the draw buffer of any queued up draws.
     */
    void endFlush() override;

    /**
     * Together these two functions flush all queued up draws to GrCommandBuffer. The return value
     * of executeOps() indicates whether any commands were actually issued to the GPU.
     */
    void onPrepare(GrOpFlushState* flushState) override;
    bool onExecute(GrOpFlushState* flushState) override;

    void addSampledTexture(GrTextureProxy* proxy) {
        fSampledProxies.push_back(proxy);
    }

    void addOp(std::unique_ptr<GrOp> op, GrTextureResolveManager textureResolveManager,
               const GrCaps& caps) {
        auto addDependency = [ textureResolveManager, &caps, this ] (
                GrTextureProxy* p, GrMipMapped mipmapped) {
            this->addDependency(p, mipmapped, textureResolveManager, caps);
        };

        op->visitProxies(addDependency);

        this->recordOp(std::move(op), GrProcessorSet::EmptySetAnalysis(), nullptr, nullptr, caps);
    }

    void addWaitOp(std::unique_ptr<GrOp> op, GrTextureResolveManager textureResolveManager,
                   const GrCaps& caps) {
        fHasWaitOp = true;
        this->addOp(std::move(op), textureResolveManager, caps);
    }

    void addDrawOp(std::unique_ptr<GrDrawOp> op, const GrProcessorSet::Analysis& processorAnalysis,
                   GrAppliedClip&& clip, const DstProxy& dstProxy,
                   GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
        auto addDependency = [ textureResolveManager, &caps, this ] (
                GrTextureProxy* p, GrMipMapped mipmapped) {
            this->addSampledTexture(p);
            this->addDependency(p, mipmapped, textureResolveManager, caps);
        };

        op->visitProxies(addDependency);
        clip.visitProxies(addDependency);
        if (dstProxy.proxy()) {
            this->addSampledTexture(dstProxy.proxy());
            addDependency(dstProxy.proxy(), GrMipMapped::kNo);
        }

        this->recordOp(std::move(op), processorAnalysis, clip.doesClip() ? &clip : nullptr,
                       &dstProxy, caps);
    }

    void discard();

    SkDEBUGCODE(void dump(bool printDependencies) const override;)
    SkDEBUGCODE(int numClips() const override { return fNumClips; })
    SkDEBUGCODE(void visitProxies_debugOnly(const VisitSurfaceProxyFunc&) const override;)

private:
    bool isNoOp() const {
        // TODO: GrLoadOp::kDiscard (i.e., storing a discard) should also be grounds for skipping
        // execution. We currently don't because of Vulkan. See http://skbug.com/9373.
        //
        // TODO: We should also consider stencil load/store here. We get away with it for now
        // because we never discard stencil buffers.
        return fOpChains.empty() && GrLoadOp::kLoad == fColorLoadOp;
    }

    void deleteOps();

    // Must only be called if native stencil buffer clearing is enabled
    void setStencilLoadOp(GrLoadOp op) { fStencilLoadOp = op; }
    // Must only be called if native color buffer clearing is enabled.
    void setColorLoadOp(GrLoadOp op, const SkPMColor4f& color);
    // Sets the clear color to transparent black
    void setColorLoadOp(GrLoadOp op) {
        static const SkPMColor4f kDefaultClearColor = {0.f, 0.f, 0.f, 0.f};
        this->setColorLoadOp(op, kDefaultClearColor);
    }

    enum class CanDiscardPreviousOps : bool {
        kYes = true,
        kNo = false
    };

    // Perform book-keeping for a fullscreen clear, regardless of how the clear is implemented later
    // (i.e. setColorLoadOp(), adding a ClearOp, or adding a GrFillRectOp that covers the device).
    // Returns true if the clear can be converted into a load op (barring device caps).
    bool resetForFullscreenClear(CanDiscardPreviousOps);

    class OpChain {
    public:
        OpChain(const OpChain&) = delete;
        OpChain& operator=(const OpChain&) = delete;
        OpChain(std::unique_ptr<GrOp>, GrProcessorSet::Analysis, GrAppliedClip*, const DstProxy*);

        ~OpChain() {
            // The ops are stored in a GrMemoryPool and must be explicitly deleted via the pool.
            SkASSERT(fList.empty());
        }

        void visitProxies(const GrOp::VisitProxyFunc&) const;

        GrOp* head() const { return fList.head(); }

        GrAppliedClip* appliedClip() const { return fAppliedClip; }
        const DstProxy& dstProxy() const { return fDstProxy; }
        const SkRect& bounds() const { return fBounds; }

        // Deletes all the ops in the chain via the pool.
        void deleteOps(GrOpMemoryPool* pool);

        // Attempts to move the ops from the passed chain to this chain at the head. Also attempts
        // to merge ops between the chains. Upon success the passed chain is empty.
        // Fails when the chains aren't of the same op type, have different clips or dst proxies.
        bool prependChain(OpChain*, const GrCaps&, GrOpMemoryPool*, GrAuditTrail*);

        // Attempts to add 'op' to this chain either by merging or adding to the tail. Returns
        // 'op' to the caller upon failure, otherwise null. Fails when the op and chain aren't of
        // the same op type, have different clips or dst proxies.
        std::unique_ptr<GrOp> appendOp(std::unique_ptr<GrOp> op, GrProcessorSet::Analysis,
                                       const DstProxy*, const GrAppliedClip*, const GrCaps&,
                                       GrOpMemoryPool*, GrAuditTrail*);

        void setSkipExecuteFlag() { fSkipExecute = true; }
        bool shouldExecute() const {
            return SkToBool(this->head()) && !fSkipExecute;
        }

    private:
        class List {
        public:
            List() = default;
            List(std::unique_ptr<GrOp>);
            List(List&&);
            List& operator=(List&& that);

            bool empty() const { return !SkToBool(fHead); }
            GrOp* head() const { return fHead.get(); }
            GrOp* tail() const { return fTail; }

            std::unique_ptr<GrOp> popHead();
            std::unique_ptr<GrOp> removeOp(GrOp* op);
            void pushHead(std::unique_ptr<GrOp> op);
            void pushTail(std::unique_ptr<GrOp>);

            void validate() const;

        private:
            std::unique_ptr<GrOp> fHead;
            GrOp* fTail = nullptr;
        };

        void validate() const;

        bool tryConcat(List*, GrProcessorSet::Analysis, const DstProxy&, const GrAppliedClip*,
                       const SkRect& bounds, const GrCaps&, GrOpMemoryPool*, GrAuditTrail*);
        static List DoConcat(List, List, const GrCaps&, GrOpMemoryPool*, GrAuditTrail*);

        List fList;
        GrProcessorSet::Analysis fProcessorAnalysis;
        DstProxy fDstProxy;
        GrAppliedClip* fAppliedClip;
        SkRect fBounds;

        // We set this flag to true if any of the ops' proxies fail to instantiate so that we know
        // not to try and draw the op.
        bool fSkipExecute = false;
    };


    bool onIsUsed(GrSurfaceProxy*) const override;

    void handleInternalAllocationFailure() override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    void recordOp(std::unique_ptr<GrOp>, GrProcessorSet::Analysis, GrAppliedClip*, const DstProxy*,
                  const GrCaps& caps);

    void forwardCombine(const GrCaps&);

    ExpectedOutcome onMakeClosed(const GrCaps& caps) override {
        this->forwardCombine(caps);
        return (this->isNoOp()) ? ExpectedOutcome::kTargetUnchanged : ExpectedOutcome::kTargetDirty;
    }

    friend class GrRenderTargetContextPriv; // for stencil clip state. TODO: this is invasive

    // The RTC and OpsTask have to work together to handle buffer clears. In most cases, buffer
    // clearing can be done natively, in which case the op list's load ops are sufficient. In other
    // cases, draw ops must be used, which makes the RTC the best place for those decisions. This,
    // however, requires that the RTC be able to coordinate with the op list to achieve similar ends
    friend class GrRenderTargetContext;

    // This is a backpointer to the GrOpMemoryPool that holds the memory for this GrOpsTask's ops.
    // In the DDL case, these back pointers keep the DDL's GrOpMemoryPool alive as long as its
    // constituent GrOpsTask survives.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrAuditTrail* fAuditTrail;

    GrLoadOp fColorLoadOp = GrLoadOp::kLoad;
    SkPMColor4f fLoadClearColor = SK_PMColor4fTRANSPARENT;
    GrLoadOp fStencilLoadOp = GrLoadOp::kLoad;

    uint32_t fLastClipStackGenID;
    SkIRect fLastDevClipBounds;
    int fLastClipNumAnalyticFPs;

    // We must track if we have a wait op so that we don't delete the op when we have a full clear.
    bool fHasWaitOp = false;;

    // For ops/opsTask we have mean: 5 stdDev: 28
    SkSTArray<25, OpChain, true> fOpChains;

    // MDB TODO: 4096 for the first allocation of the clip space will be huge overkill.
    // Gather statistics to determine the correct size.
    SkArenaAlloc fClipAllocator{4096};
    SkDEBUGCODE(int fNumClips;)

    // TODO: We could look into this being a set if we find we're adding a lot of duplicates that is
    // causing slow downs.
    SkTArray<GrTextureProxy*, true> fSampledProxies;
};

#endif
