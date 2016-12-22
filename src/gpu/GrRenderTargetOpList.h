/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetOpList_DEFINED
#define GrRenderTargetOpList_DEFINED

#include "GrClip.h"
#include "GrContext.h"
#include "GrOpList.h"
#include "GrPathProcessor.h"
#include "GrPrimitiveProcessor.h"
#include "GrPathRendering.h"
#include "GrXferProcessor.h"

#include "ops/GrDrawOp.h"

#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"

class GrAuditTrail;
class GrClearOp;
class GrClip;
class GrCaps;
class GrPath;
class GrDrawPathOpBase;
class GrOp;
class GrPipelineBuilder;
class GrRenderTargetProxy;

class GrRenderTargetOpList final : public GrOpList {
public:
    /** Options for GrRenderTargetOpList behavior. */
    struct Options {
        bool fClipDrawOpsToBounds = false;
        int fMaxOpCombineLookback = -1;
        int fMaxOpCombineLookahead = -1;
    };

    GrRenderTargetOpList(GrRenderTargetProxy*, GrGpu*, GrResourceProvider*,
                         GrAuditTrail*, const Options&);

    ~GrRenderTargetOpList() override;

    void makeClosed() override {
        INHERITED::makeClosed();

        fLastFullClearOp = nullptr;
        this->forwardCombine();
    }

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

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fGpu->caps(); }

    void addDrawOp(const GrPipelineBuilder&, GrRenderTargetContext*, const GrClip&,
                   sk_sp<GrDrawOp>);

    void addOp(sk_sp<GrOp> op, GrRenderTargetContext* renderTargetContext) {
        this->recordOp(std::move(op), renderTargetContext);
    }

    /**
     * Draws the path into user stencil bits. Upon return, all user stencil values
     * inside the path will be nonzero. The path's fill must be either even/odd or
     * winding (notnverse or hairline).It will respect the HW antialias boolean (if
     * possible in the 3D API).  Note, we will never have an inverse fill with
     * stencil path.
     */
    void stencilPath(GrRenderTargetContext*,
                     const GrClip&,
                     GrAAType aa,
                     const SkMatrix& viewMatrix,
                     const GrPath*);

    /** Clears the entire render target */
    void fullClear(GrRenderTargetContext*, GrColor color);

    /** Discards the contents render target. */
    void discard(GrRenderTargetContext*);

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
    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    gr_instanced::InstancedRendering* instancedRendering() const {
        SkASSERT(fInstancedRendering);
        return fInstancedRendering.get();
    }

    GrRenderTargetOpList* asRenderTargetOpList() override { return this; }

    SkDEBUGCODE(void dump() const override;)

private:
    friend class GrRenderTargetContextPriv; // for clearStencilClip and stencil clip state.

    // If the input op is combined with an earlier op, this returns the combined op. Otherwise, it
    // returns the input op.
    GrOp* recordOp(sk_sp<GrOp> op, GrRenderTargetContext* renderTargetContext) {
        SkRect bounds = op->bounds();
        return this->recordOp(std::move(op), renderTargetContext, bounds);
    }

    // Variant that allows an explicit bounds (computed from the Op's bounds and a clip).
    GrOp* recordOp(sk_sp<GrOp>, GrRenderTargetContext*, const SkRect& clippedBounds);

    void forwardCombine();

    // Makes a copy of the dst if it is necessary for the draw and returns the texture that should
    // be used by GrXferProcessor to access the destination color. If the texture is nullptr then
    // a texture copy could not be made.
    void setupDstTexture(GrRenderTarget*,
                         const GrClip&,
                         const SkRect& opBounds,
                         GrXferProcessor::DstTexture*);

    // Used only via GrRenderTargetContextPriv.
    void clearStencilClip(const GrFixedClip&, bool insideStencilMask, GrRenderTargetContext*);

    struct RecordedOp {
        sk_sp<GrOp> fOp;
        SkRect fClippedBounds;
        // TODO: Use proxy ID instead of instantiated render target ID.
        GrGpuResource::UniqueID fRenderTargetID;
    };
    SkSTArray<256, RecordedOp, true> fRecordedOps;

    GrClearOp* fLastFullClearOp = nullptr;
    GrGpuResource::UniqueID fLastFullClearRenderTargetID = GrGpuResource::UniqueID::InvalidID();

    // The context is only in service of the GrClip, remove once it doesn't need this.
    GrContext* fContext;
    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;

    bool fClipOpToBounds;
    int fMaxOpLookback;
    int fMaxOpLookahead;

    std::unique_ptr<gr_instanced::InstancedRendering> fInstancedRendering;

    int32_t fLastClipStackGenID;
    SkIRect fLastClipStackRect;
    SkIPoint fLastClipOrigin;

    typedef GrOpList INHERITED;
};

#endif
