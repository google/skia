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
#include "GrTraceMarker.h"
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

class GrClip;
class GrCaps;
class GrPath;
class GrPathRange;
class GrPipeline;

class GrDrawTarget : public SkRefCnt {
public:
    

    typedef GrPathRange::PathIndexType PathIndexType;
    typedef GrPathRendering::PathTransformType PathTransformType;

    ///////////////////////////////////////////////////////////////////////////

    // The context may not be fully constructed and should not be used during GrDrawTarget
    // construction.
    GrDrawTarget(GrGpu* gpu, GrResourceProvider*);

    virtual ~GrDrawTarget();

    /**
     * Empties the draw buffer of any queued up draws.
     */
    void reset() { this->onReset(); }

    /**
     * This plays any queued up draws to its GrGpu target. It also resets this object (i.e. flushing
     * is destructive).
     */
    void flush();

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fCaps; }

    void drawBatch(const GrPipelineBuilder&, GrBatch*);

    /**
     * Draws path into the stencil buffer. The fill must be either even/odd or
     * winding (not inverse or hairline). It will respect the HW antialias flag
     * on the GrPipelineBuilder (if possible in the 3D API).  Note, we will never have an inverse
     * fill with stencil path
     */
    void stencilPath(const GrPipelineBuilder&, const GrPathProcessor*, const GrPath*,
                     GrPathRendering::FillType);

    /**
     * Draws a path. Fill must not be a hairline. It will respect the HW
     * antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     */
    void drawPath(const GrPipelineBuilder&, const GrPathProcessor*, const GrPath*,
                  GrPathRendering::FillType);

    /**
     * Draws the aggregate path from combining multiple. Note that this will not
     * always be equivalent to back-to-back calls to drawPath(). It will respect
     * the HW antialias flag on the GrPipelineBuilder (if possible in the 3D API).
     *
     * @param pathRange       Source paths to draw from
     * @param indices         Array of path indices to draw
     * @param indexType       Data type of the array elements in indexBuffer
     * @param transformValues Array of transforms for the individual paths
     * @param transformType   Type of transforms in transformBuffer
     * @param count           Number of paths to draw
     * @param fill            Fill type for drawing all the paths
     */
    void drawPaths(const GrPipelineBuilder&,
                   const GrPathProcessor*,
                   const GrPathRange* pathRange,
                   const void* indices,
                   PathIndexType indexType,
                   const float transformValues[],
                   PathTransformType transformType,
                   int count,
                   GrPathRendering::FillType fill);

    /**
     * Helper function for drawing rects.
     *
     * @param rect        the rect to draw
     * @param localRect   optional rect that specifies local coords to map onto
     *                    rect. If NULL then rect serves as the local coords.
     * @param localMatrix Optional local matrix. The local coordinates are specified by localRect,
     *                    or if it is NULL by rect. This matrix applies to the coordinate implied by
     *                    that rectangle before it is input to GrCoordTransforms that read local
     *                    coordinates
     */
    void drawBWRect(const GrPipelineBuilder& pipelineBuilder,
                    GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix);

    /**
     * Helper for drawRect when the caller doesn't need separate local rects or matrices.
     */
    void drawSimpleRect(const GrPipelineBuilder& ds, GrColor color, const SkMatrix& viewM,
                        const SkRect& rect) {
        this->drawBWRect(ds, color, viewM, rect, NULL, NULL);
    }
    void drawSimpleRect(const GrPipelineBuilder& ds, GrColor color, const SkMatrix& viewM,
                        const SkIRect& irect) {
        SkRect rect = SkRect::Make(irect);
        this->drawBWRect(ds, color, viewM, rect, NULL, NULL);
    }

    void drawAARect(const GrPipelineBuilder& pipelineBuilder,
                    GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect);

    /**
     * Clear the passed in render target. Ignores the GrPipelineBuilder and clip. Clears the whole
     * thing if rect is NULL, otherwise just the rect. If canIgnoreRect is set then the entire
     * render target can be optionally cleared.
     */
    void clear(const SkIRect* rect,
               GrColor color,
               bool canIgnoreRect,
               GrRenderTarget* renderTarget);

    /**
     * Discards the contents render target.
     **/
    virtual void discard(GrRenderTarget*) = 0;

    /**
     * Called at start and end of gpu trace marking
     * GR_CREATE_GPU_TRACE_MARKER(marker_str, target) will automatically call these at the start
     * and end of a code block respectively
     */
    void addGpuTraceMarker(const GrGpuTraceMarker* marker);
    void removeGpuTraceMarker(const GrGpuTraceMarker* marker);

    /**
     * Takes the current active set of markers and stores them for later use. Any current marker
     * in the active set is removed from the active set and the targets remove function is called.
     * These functions do not work as a stack so you cannot call save a second time before calling
     * restore. Also, it is assumed that when restore is called the current active set of markers
     * is empty. When the stored markers are added back into the active set, the targets add marker
     * is called.
     */
    void saveActiveTraceMarkers();
    void restoreActiveTraceMarkers();

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
     */
    virtual void purgeResources() {};

    bool programUnitTest(GrContext* owner, int maxStages);

protected:
    friend class GrCommandBuilder; // for PipelineInfo
    friend class GrInOrderCommandBuilder; // for PipelineInfo
    friend class GrReorderCommandBuilder; // for PipelineInfo
    friend class GrTargetCommands; // for PipelineInfo

    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const { return fGpu; }

    const GrTraceMarkerSet& getActiveTraceMarkers() { return fActiveTraceMarkers; }

    // Makes a copy of the dst if it is necessary for the draw. Returns false if a copy is required
    // but couldn't be made. Otherwise, returns true.  This method needs to be protected because it
    // needs to be accessed by GLPrograms to setup a correct drawstate
    bool setupDstReadIfNecessary(const GrPipelineBuilder&,
                                 const GrProcOptInfo& colorPOI,
                                 const GrProcOptInfo& coveragePOI,
                                 GrXferProcessor::DstTexture*,
                                 const SkRect* drawBounds);

    struct PipelineInfo {
        PipelineInfo(const GrPipelineBuilder& pipelineBuilder, GrScissorState* scissor,
                     const GrPrimitiveProcessor* primProc,
                     const SkRect* devBounds, GrDrawTarget* target);

        PipelineInfo(const GrPipelineBuilder& pipelineBuilder, GrScissorState* scissor,
                     const GrBatch* batch, const SkRect* devBounds,
                     GrDrawTarget* target);

        bool willColorBlendWithDst(const GrPrimitiveProcessor* primProc) const {
            return fPipelineBuilder->willColorBlendWithDst(primProc);
        }
    private:
        friend class GrDrawTarget;

        bool mustSkipDraw() const { return (NULL == fPipelineBuilder); }

        const GrPipelineBuilder*    fPipelineBuilder;
        GrScissorState*             fScissor;
        GrProcOptInfo               fColorPOI; 
        GrProcOptInfo               fCoveragePOI; 
        GrXferProcessor::DstTexture fDstTexture;
    };

    const GrPipeline* setupPipeline(const PipelineInfo& pipelineInfo, void* pipelineAddr);

private:
    virtual void onReset() = 0;

    virtual void onFlush() = 0;

    virtual void onDrawBatch(GrBatch*, const PipelineInfo&) = 0;
    virtual void onStencilPath(const GrPipelineBuilder&,
                               const GrPathProcessor*,
                               const GrPath*,
                               const GrScissorState&,
                               const GrStencilSettings&) = 0;
    virtual void onDrawPath(const GrPathProcessor*,
                            const GrPath*,
                            const GrStencilSettings&,
                            const PipelineInfo&) = 0;
    virtual void onDrawPaths(const GrPathProcessor*,
                             const GrPathRange*,
                             const void* indices,
                             PathIndexType,
                             const float transformValues[],
                             PathTransformType,
                             int count,
                             const GrStencilSettings&,
                             const PipelineInfo&) = 0;

    virtual void onClear(const SkIRect& rect, GrColor color, GrRenderTarget* renderTarget) = 0;

    /** The subclass's copy surface implementation. It should assume that any clipping has already
        been performed on the rect and point and that the GrGpu supports the copy. */
    virtual void onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    // Check to see if this set of draw commands has been sent out
    virtual bool       isIssued(uint32_t drawID) { return true; }
    void getPathStencilSettingsForFilltype(GrPathRendering::FillType,
                                           const GrStencilAttachment*,
                                           GrStencilSettings*);
    virtual GrClipMaskManager* clipMaskManager() = 0;
    virtual bool setupClip(const GrPipelineBuilder&,
                           GrPipelineBuilder::AutoRestoreFragmentProcessorState*,
                           GrPipelineBuilder::AutoRestoreStencil*,
                           GrScissorState*,
                           const SkRect* devBounds) = 0;

    GrGpu*                  fGpu;
    const GrCaps*           fCaps;
    GrResourceProvider*     fResourceProvider;
    // To keep track that we always have at least as many debug marker adds as removes
    int                     fGpuTraceMarkerCount;
    GrTraceMarkerSet        fActiveTraceMarkers;
    GrTraceMarkerSet        fStoredTraceMarkers;
    bool                    fFlushing;

    typedef SkRefCnt INHERITED;
};

/*
 * This class is JUST for clip mask manager.  Everyone else should just use draw target above.
 */
class GrClipTarget : public GrDrawTarget {
public:
    GrClipTarget(GrContext*);

    /* Clip mask manager needs access to the context.
     * TODO we only need a very small subset of context in the CMM.
     */
    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    /**
     * Clip Mask Manager(and no one else) needs to clear private stencil bits.
     * ClipTarget subclass sets clip bit in the stencil buffer. The subclass
     * is free to clear the remaining bits to zero if masked clears are more
     * expensive than clearing all bits.
     */
    virtual void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* = NULL) = 0;

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    void purgeResources() override;

protected:
    SkAutoTDelete<GrClipMaskManager> fClipMaskManager;
    GrContext*                       fContext;

private:
    GrClipMaskManager* clipMaskManager() override { return fClipMaskManager; }

    bool setupClip(const GrPipelineBuilder&,
                   GrPipelineBuilder::AutoRestoreFragmentProcessorState*,
                   GrPipelineBuilder::AutoRestoreStencil*,
                   GrScissorState* scissorState,
                   const SkRect* devBounds) override;

    typedef GrDrawTarget INHERITED;
};

#endif
