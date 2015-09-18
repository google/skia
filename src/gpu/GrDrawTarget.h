/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrClip.h"
#include "GrClipMaskManager.h"
#include "GrContext.h"
#include "GrPathProcessor.h"
#include "GrPrimitiveProcessor.h"
#include "GrIndexBuffer.h"
#include "GrPathRendering.h"
#include "GrPipelineBuilder.h"
#include "GrPipeline.h"
#include "GrVertexBuffer.h"
#include "GrXferProcessor.h"

#include "SkClipStack.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"
#include "SkXfermode.h"

class GrBatch;
class GrClip;
class GrCaps;
class GrPath;
class GrDrawBatch;
class GrDrawPathBatchBase;
class GrPathRangeDraw;

class GrDrawTarget final : public SkRefCnt {
public:
    // The context may not be fully constructed and should not be used during GrDrawTarget
    // construction.
    GrDrawTarget(GrGpu* gpu, GrResourceProvider*);

    ~GrDrawTarget() override;

    /**
     * Empties the draw buffer of any queued up draws.
     */
    void reset();

    /**
     * This plays any queued up draws to its GrGpu target. It also resets this object (i.e. flushing
     * is destructive).
     */
    void flush();

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fGpu->caps(); }

    void drawBatch(const GrPipelineBuilder&, GrDrawBatch*);

    /**
     * Draws path into the stencil buffer. The fill must be either even/odd or
     * winding (not inverse or hairline). It will respect the HW antialias flag
     * on the GrPipelineBuilder (if possible in the 3D API).  Note, we will never have an inverse
     * fill with stencil path
     */
    void stencilPath(const GrPipelineBuilder&, const SkMatrix& viewMatrix, const GrPath*,
                     GrPathRendering::FillType);

    /**
     * Draws a path. Fill must not be a hairline. It will respect the HW
     * antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     *
     * TODO: Remove this function and construct the batch outside GrDrawTarget.
     */
    void drawPath(const GrPipelineBuilder&, const SkMatrix& viewMatrix, GrColor color,
                  const GrPath*, GrPathRendering::FillType);

    /**
     * Draws the aggregate path from combining multiple. Note that this will not
     * always be equivalent to back-to-back calls to drawPath(). It will respect
     * the HW antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     *
     * TODO: Remove this function and construct the batch outside GrDrawTarget.
     *
     * @param draw            The range, transforms, and indices for the draw.
     *                        This object must only be drawn once. The draw
     *                        may modify its contents.
     * @param fill            Fill type for drawing all the paths
     */
    void drawPathsFromRange(const GrPipelineBuilder&,
                            const SkMatrix& viewMatrix,
                            const SkMatrix& localMatrix,
                            GrColor color,
                            GrPathRangeDraw* draw,
                            GrPathRendering::FillType fill);

    /**
     * Helper function for drawing rects.
     *
     * @param rect        the rect to draw
     * @param localRect   optional rect that specifies local coords to map onto
     *                    rect. If nullptr then rect serves as the local coords.
     * @param localMatrix Optional local matrix. The local coordinates are specified by localRect,
     *                    or if it is nullptr by rect. This matrix applies to the coordinate implied by
     *                    that rectangle before it is input to GrCoordTransforms that read local
     *                    coordinates
     */
    void drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                       GrColor color,
                       const SkMatrix& viewMatrix,
                       const SkRect& rect);

    void drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                       GrColor color,
                       const SkMatrix& viewMatrix,
                       const SkRect& rect,
                       const SkMatrix& localMatrix);

    void drawNonAARect(const GrPipelineBuilder& pipelineBuilder,
                       GrColor color,
                       const SkMatrix& viewMatrix,
                       const SkRect& rect,
                       const SkRect& localRect);

    void drawNonAARect(const GrPipelineBuilder& ds,
                       GrColor color,
                       const SkMatrix& viewM,
                       const SkIRect& irect) {
        SkRect rect = SkRect::Make(irect);
        this->drawNonAARect(ds, color, viewM, rect);
    }

    void drawAARect(const GrPipelineBuilder& pipelineBuilder,
                    GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect);

    /**
     * Clear the passed in render target. Ignores the GrPipelineBuilder and clip. Clears the whole
     * thing if rect is nullptr, otherwise just the rect. If canIgnoreRect is set then the entire
     * render target can be optionally cleared.
     */
    void clear(const SkIRect* rect,
               GrColor color,
               bool canIgnoreRect,
               GrRenderTarget* renderTarget);

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
    void copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);
    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     * TODO: Stop holding on to resources.
     */
    virtual void purgeResources() {
        // The clip mask manager can rebuild all its clip masks so just get rid of them all.
        fClipMaskManager->purgeResources();
    };

    bool programUnitTest(GrContext* owner, int maxStages);

    /** Provides access to internal functions to GrClipMaskManager without friending all of
        GrDrawTarget to CMM. */
    class CMMAccess {
    public:
        CMMAccess(GrDrawTarget* drawTarget) : fDrawTarget(drawTarget) {}
    private:
        void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* rt) const {
            fDrawTarget->clearStencilClip(rect, insideClip, rt);
        }

        GrContext* context() const { return fDrawTarget->fContext; }
        GrResourceProvider* resourceProvider() const { return fDrawTarget->fResourceProvider; }
        GrDrawTarget* fDrawTarget;
        friend class GrClipMaskManager;
    };

    const CMMAccess cmmAccess() { return CMMAccess(this); }

private:
    void recordBatch(GrBatch*);
    bool installPipelineInDrawBatch(const GrPipelineBuilder* pipelineBuilder,
                                    const GrScissorState* scissor,
                                    GrDrawBatch* batch);

    // Makes a copy of the dst if it is necessary for the draw. Returns false if a copy is required
    // but couldn't be made. Otherwise, returns true.  This method needs to be protected because it
    // needs to be accessed by GLPrograms to setup a correct drawstate
    bool setupDstReadIfNecessary(const GrPipelineBuilder&,
        const GrProcOptInfo& colorPOI,
        const GrProcOptInfo& coveragePOI,
        GrXferProcessor::DstTexture*,
        const SkRect& batchBounds);

    void drawPathBatch(const GrPipelineBuilder& pipelineBuilder, GrDrawPathBatchBase* batch,
                       GrPathRendering::FillType fill);
    // Check to see if this set of draw commands has been sent out
    void getPathStencilSettingsForFilltype(GrPathRendering::FillType,
                                           const GrStencilAttachment*,
                                           GrStencilSettings*);
    bool setupClip(const GrPipelineBuilder&,
                           GrPipelineBuilder::AutoRestoreFragmentProcessorState*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState*,
                           const SkRect* devBounds);

    // Used only by CMM.
    void clearStencilClip(const SkIRect&, bool insideClip, GrRenderTarget*);

    SkSTArray<256, SkAutoTUnref<GrBatch>, true> fBatches;
    SkAutoTDelete<GrClipMaskManager>            fClipMaskManager;
    // The context is only in service of the clip mask manager, remove once CMM doesn't need this.
    GrContext*                                  fContext;
    GrGpu*                                      fGpu;
    GrResourceProvider*                         fResourceProvider;
    bool                                        fFlushing;
    GrBatchToken                                fLastFlushToken;

    typedef SkRefCnt INHERITED;
};

#endif
