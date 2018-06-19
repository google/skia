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

    bool isEmpty() const { return fRecordedOps.empty(); }

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

    /**
     * Returns this opList's id if the Op was recorded, or SK_InvalidUniqueID if it was combined
     * into an existing Op or otherwise deleted.
     */
    uint32_t addOp(std::unique_ptr<GrOp> op, const GrCaps& caps) {
        auto addDependency = [ &caps, this ] (GrSurfaceProxy* p) {
            this->addDependency(p, caps);
        };

        op->visitProxies(addDependency);

        return this->recordOp(std::move(op), caps);
    }

    /**
     * Returns this opList's id if the Op was recorded, or SK_InvalidUniqueID if it was combined
     * into an existing Op or otherwise deleted.
     */
    uint32_t addOp(std::unique_ptr<GrOp> op, const GrCaps& caps,
                   GrAppliedClip&& clip, const DstProxy& dstProxy) {
        auto addDependency = [ &caps, this ] (GrSurfaceProxy* p) {
            this->addDependency(p, caps);
        };

        op->visitProxies(addDependency);
        clip.visitProxies(addDependency);

        return this->recordOp(std::move(op), caps, clip.doesClip() ? &clip : nullptr, &dstProxy);
    }

    void discard();

    /** Clears the entire render target */
    void fullClear(GrContext*, GrColor color);

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

    SkDEBUGCODE(int numOps() const override { return fRecordedOps.count(); })
    SkDEBUGCODE(int numClips() const override { return fNumClips; })
    SkDEBUGCODE(void visitProxies_debugOnly(const GrOp::VisitProxyFunc&) const;)

private:
    friend class GrRenderTargetContextPriv; // for stencil clip state. TODO: this is invasive

    void deleteOps();

    struct RecordedOp {
        RecordedOp(std::unique_ptr<GrOp> op, GrAppliedClip* appliedClip, const DstProxy* dstProxy)
                : fOp(std::move(op)), fAppliedClip(appliedClip) {
            if (dstProxy) {
                fDstProxy = *dstProxy;
            }
        }

        ~RecordedOp() {
            // The ops are stored in a GrMemoryPool so had better have been handled separately
            SkASSERT(!fOp);
        }

        void deleteOp(GrOpMemoryPool* opMemoryPool);

        void visitProxies(const GrOp::VisitProxyFunc& func) const {
            if (fOp) {
                fOp->visitProxies(func);
            }
            if (fDstProxy.proxy()) {
                func(fDstProxy.proxy());
            }
            if (fAppliedClip) {
                fAppliedClip->visitProxies(func);
            }
        }

        std::unique_ptr<GrOp> fOp;
        DstProxy              fDstProxy;
        GrAppliedClip*        fAppliedClip;
    };

    void purgeOpsWithUninstantiatedProxies() override;

    void gatherProxyIntervals(GrResourceAllocator*) const override;

    // Returns this opList's id if the Op was recorded, or SK_InvalidUniqueID if it was combined
    // into an existing Op or otherwise deleted.
    uint32_t recordOp(std::unique_ptr<GrOp>, const GrCaps& caps,
                      GrAppliedClip* = nullptr, const DstProxy* = nullptr);

    void forwardCombine(const GrCaps&);

    // If this returns true then b has been merged into a's op.
    bool combineIfPossible(const RecordedOp& a, GrOp* b, const GrAppliedClip* bClip,
                           const DstProxy* bDstTexture, const GrCaps&);

    uint32_t                       fLastClipStackGenID;
    SkIRect                        fLastDevClipBounds;
    int                            fLastClipNumAnalyticFPs;

    // For ops/opList we have mean: 5 stdDev: 28
    SkSTArray<5, RecordedOp, true> fRecordedOps;

    // MDB TODO: 4096 for the first allocation of the clip space will be huge overkill.
    // Gather statistics to determine the correct size.
    SkArenaAlloc                   fClipAllocator{4096};
    SkDEBUGCODE(int                fNumClips;)

    typedef GrOpList INHERITED;
};

#endif
