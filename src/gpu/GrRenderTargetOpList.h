/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetOpList_DEFINED
#define GrRenderTargetOpList_DEFINED

#include "GrAppliedClip.h"
#include "GrOpList.h"
#include "GrPathRendering.h"
#include "GrPrimitiveProcessor.h"
#include "ops/GrOp.h"
#include "SkArenaAlloc.h"
#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"

class GrAuditTrail;
class GrClearOp;
class GrCaps;
class GrRenderTargetProxy;

class GrRenderTargetOpList final : public GrOpList {
private:
    using DstProxy = GrXferProcessor::DstProxy;

public:
    GrRenderTargetOpList(GrResourceProvider*, sk_sp<GrOpMemoryPool>,
                         GrRenderTargetProxy*, GrAuditTrail*);

    ~GrRenderTargetOpList() override;

    void makeClosed(const GrCaps& caps) override {
        if (this->isClosed()) {
            return;
        }

        this->forwardCombine(caps);

        INHERITED::makeClosed(caps);
    }

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

    void addOp(std::unique_ptr<GrOp> op, const GrCaps& caps) {
        auto addDependency = [ &caps, this ] (GrSurfaceProxy* p) {
            this->addDependency(p, caps);
        };

        op->visitProxies(addDependency);

        this->recordOp(std::move(op), caps);
    }

    void addOp(std::unique_ptr<GrOp> op, const GrCaps& caps, GrAppliedClip&& clip,
               const DstProxy& dstProxy) {
        auto addDependency = [ &caps, this ] (GrSurfaceProxy* p) {
            this->addDependency(p, caps);
        };

        op->visitProxies(addDependency);
        clip.visitProxies(addDependency);
        if (dstProxy.proxy()) {
            addDependency(dstProxy.proxy());
        }

        this->recordOp(std::move(op), caps, clip.doesClip() ? &clip : nullptr, &dstProxy);
    }

    void discard();

    /** Clears the entire render target */
    void fullClear(GrContext*, const SkPMColor4f& color);

    /**
     * Copies a pixel rectangle from one surface to another. This call may finalize
     * reserved vertex/index data (as though a draw call was made). The src pixels
     * copied are specified by srcRect. They are copied to a rect of the same
     * size in dst with top left at dstPoint. If the src rect is clipped by the
     * src bounds then  pixel values in the dst rect corresponding to area clipped
     * by the src rect are not overwritten. This method is not guaranteed to succeed
     * depending on the type of surface, configs, etc, and the backend-specific
     * limitations.
     */
    bool copySurface(GrContext*,
                     GrSurfaceProxy* dst,
                     GrSurfaceProxy* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override;

    GrRenderTargetOpList* asRenderTargetOpList() override { return this; }

    SkDEBUGCODE(void dump(bool printDependencies) const override;)
    SkDEBUGCODE(int numClips() const override { return fNumClips; })
    SkDEBUGCODE(void visitProxies_debugOnly(const GrOp::VisitProxyFunc&) const;)

private:
    friend class GrRenderTargetContextPriv; // for stencil clip state. TODO: this is invasive

    void deleteOps();

    class OpChain {
    public:
        OpChain(const OpChain&) = delete;
        OpChain& operator=(const OpChain&) = delete;
        OpChain(std::unique_ptr<GrOp>, GrAppliedClip*, const DstProxy*);

        ~OpChain() {
            // The ops are stored in a GrMemoryPool and must be explicitly deleted via the pool.
            SkASSERT(fList.empty());
        }

        void visitProxies(const GrOp::VisitProxyFunc&, GrOp::VisitorType) const;

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
        std::unique_ptr<GrOp> appendOp(std::unique_ptr<GrOp> op, const DstProxy*,
                                       const GrAppliedClip*, const GrCaps&, GrOpMemoryPool*,
                                       GrAuditTrail*);

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

        std::tuple<List, List> TryConcat(List chainA, const DstProxy& dstProxyA,
                                         const GrAppliedClip* appliedClipA, List chainB,
                                         const DstProxy& dstProxyB,
                                         const GrAppliedClip* appliedClipB, const GrCaps&,
                                         GrOpMemoryPool*, GrAuditTrail*);
        List DoConcat(List, List, const GrCaps&, GrOpMemoryPool*, GrAuditTrail*);

        List fList;
        DstProxy fDstProxy;
        GrAppliedClip* fAppliedClip;
        SkRect fBounds;
    };

    void purgeOpsWithUninstantiatedProxies() override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    void recordOp(std::unique_ptr<GrOp>, const GrCaps& caps, GrAppliedClip* = nullptr,
                  const DstProxy* = nullptr);

    void forwardCombine(const GrCaps&);

    uint32_t                       fLastClipStackGenID;
    SkIRect                        fLastDevClipBounds;
    int                            fLastClipNumAnalyticFPs;

    // For ops/opList we have mean: 5 stdDev: 28
    SkSTArray<25, OpChain, true> fOpChains;

    // MDB TODO: 4096 for the first allocation of the clip space will be huge overkill.
    // Gather statistics to determine the correct size.
    SkArenaAlloc                   fClipAllocator{4096};
    SkDEBUGCODE(int                fNumClips;)

    typedef GrOpList INHERITED;
};

#endif
