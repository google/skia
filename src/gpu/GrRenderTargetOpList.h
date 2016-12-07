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

#include "batches/GrDrawOp.h"

#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"

class GrAuditTrail;
class GrClearBatch;
class GrClip;
class GrCaps;
class GrPath;
class GrDrawPathBatchBase;
class GrOp;
class GrPipelineBuilder;
class GrRenderTargetProxy;

class GrRenderTargetOpList final : public GrOpList {
public:
    /** Options for GrRenderTargetOpList behavior. */
    struct Options {
        Options ()
            : fClipBatchToBounds(false)
            , fMaxBatchLookback(-1)
            , fMaxBatchLookahead(-1) {}
        bool fClipBatchToBounds;
        int  fMaxBatchLookback;
        int  fMaxBatchLookahead;
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
    void prepareOps(GrBatchFlushState* flushState) override;
    bool executeOps(GrBatchFlushState* flushState) override;

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fGpu->caps(); }

    void addDrawOp(const GrPipelineBuilder&, GrRenderTargetContext*, const GrClip&, GrDrawOp*);

    void addOp(sk_sp<GrOp>);

    /**
     * Draws the path into user stencil bits. Upon return, all user stencil values
     * inside the path will be nonzero. The path's fill must be either even/odd or
     * winding (notnverse or hairline).It will respect the HW antialias boolean (if
     * possible in the 3D API).  Note, we will never have an inverse fill with
     * stencil path.
     */
    void stencilPath(GrRenderTargetContext*,
                     const GrClip&,
                     bool useHWAA,
                     const SkMatrix& viewMatrix,
                     const GrPath*);

    /** Clears the entire render target */
    void fullClear(GrRenderTarget*, GrColor color);

    /** Discards the contents render target. */
    void discard(GrRenderTarget*);

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
    GrOp* recordOp(GrOp*, const SkRect& clippedBounds);
    void forwardCombine();

    // Makes a copy of the dst if it is necessary for the draw and returns the texture that should
    // be used by GrXferProcessor to access the destination color. If the texture is nullptr then
    // a texture copy could not be made.
    void setupDstTexture(GrRenderTarget*,
                         const GrClip&,
                         const SkRect& drawOpBounds,
                         GrXferProcessor::DstTexture*);

    // Used only via GrRenderTargetContextPriv.
    void clearStencilClip(const GrFixedClip&, bool insideStencilMask, GrRenderTarget*);

    struct RecordedOp {
        sk_sp<GrOp> fOp;
        SkRect fClippedBounds;
    };
    SkSTArray<256, RecordedOp, true>                fRecordedOps;
    GrClearBatch*                                   fLastFullClearOp;
    // The context is only in service of the GrClip, remove once it doesn't need this.
    GrContext*                                      fContext;
    GrGpu*                                          fGpu;
    GrResourceProvider*                             fResourceProvider;

    bool                                            fClipOpToBounds;
    int                                             fMaxOpLookback;
    int                                             fMaxOpLookahead;

    std::unique_ptr<gr_instanced::InstancedRendering> fInstancedRendering;

    int32_t                                         fLastClipStackGenID;
    SkIRect                                         fLastClipStackRect;
    SkIPoint                                        fLastClipOrigin;

    typedef GrOpList INHERITED;
};

#endif
