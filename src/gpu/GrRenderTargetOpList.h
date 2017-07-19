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

namespace gr_instanced {
    class InstancedRendering;
}

class GrRenderTargetOpList final : public GrOpList {
private:
    using DstProxy = GrXferProcessor::DstProxy;

public:
    GrRenderTargetOpList(GrRenderTargetProxy*, GrGpu*, GrAuditTrail*);

    ~GrRenderTargetOpList() override;

    void makeClosed(const GrCaps& caps) override {
        if (this->isClosed()) {
            return;
        }

        fLastFullClearOp = nullptr;
        this->forwardCombine(caps);

        INHERITED::makeClosed(caps);
    }

    bool isEmpty() const { return fRecordedOps.empty(); }

    /**
     * Empties the draw buffer of any queued up draws.
     */
    void reset() override;

    void abandonGpuResources() override;
    void freeGpuResources() override;

    /**
     * Together these two functions flush all queued up draws to GrCommandBuffer. The return value
     * of executeOps() indicates whether any commands were actually issued to the GPU.
     */
    void prepareOps(GrOpFlushState* flushState) override;
    bool executeOps(GrOpFlushState* flushState) override;

    uint32_t addOp(std::unique_ptr<GrOp> op, const GrCaps& caps) {
        this->recordOp(std::move(op), caps, nullptr, nullptr);
        return this->uniqueID();
    }
    uint32_t addOp(std::unique_ptr<GrOp> op, const GrCaps& caps,
                   GrAppliedClip&& clip, const DstProxy& dstProxy) {
        this->recordOp(std::move(op), caps, clip.doesClip() ? &clip : nullptr, &dstProxy);
        return this->uniqueID();
    }

    /** Clears the entire render target */
    void fullClear(const GrCaps& caps, GrColor color);

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
    bool copySurface(const GrCaps& caps,
                     GrSurfaceProxy* dst,
                     GrSurfaceProxy* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override;

    gr_instanced::InstancedRendering* instancedRendering() const {
        SkASSERT(fInstancedRendering);
        return fInstancedRendering.get();
    }

    GrRenderTargetOpList* asRenderTargetOpList() override { return this; }

    SkDEBUGCODE(void dump() const override;)

    SkDEBUGCODE(int numOps() const override { return fRecordedOps.count(); })
    SkDEBUGCODE(int numClips() const override { return fNumClips; })

private:
    friend class GrRenderTargetContextPriv; // for stencil clip state. TODO: this is invasive

    struct RecordedOp {
        RecordedOp(std::unique_ptr<GrOp> op,
                   const GrAppliedClip* appliedClip,
                   const DstProxy* dstProxy)
                : fOp(std::move(op))
                , fAppliedClip(appliedClip) {
            if (dstProxy) {
                fDstProxy = *dstProxy;
            }
        }
        std::unique_ptr<GrOp> fOp;
        DstProxy              fDstProxy;
        const GrAppliedClip*  fAppliedClip;
    };

    // If the input op is combined with an earlier op, this returns the combined op. Otherwise, it
    // returns the input op.
    GrOp* recordOp(std::unique_ptr<GrOp>, const GrCaps& caps,
                   GrAppliedClip* = nullptr, const DstProxy* = nullptr);

    void forwardCombine(const GrCaps&);

    // If this returns true then b has been merged into a's op.
    bool combineIfPossible(const RecordedOp& a, GrOp* b, const GrAppliedClip* bClip,
                           const DstProxy* bDstTexture, const GrCaps&);

    GrClearOp*                     fLastFullClearOp = nullptr;

    std::unique_ptr<gr_instanced::InstancedRendering> fInstancedRendering;

    uint32_t                       fLastClipStackGenID;
    SkIRect                        fLastDevClipBounds;

    // For ops/opList we have mean: 5 stdDev: 28
    SkSTArray<5, RecordedOp, true> fRecordedOps;

    // MDB TODO: 4096 for the first allocation of the clip space will be huge overkill.
    // Gather statistics to determine the correct size.
    SkArenaAlloc                   fClipAllocator{4096};
    SkDEBUGCODE(int                fNumClips;)

    typedef GrOpList INHERITED;
};

#endif
