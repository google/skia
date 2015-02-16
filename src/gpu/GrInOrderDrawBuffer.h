/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrFlushToGpuDrawTarget.h"

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrPipeline.h"
#include "GrPath.h"
#include "GrTRecorder.h"

/**
 * GrInOrderDrawBuffer is an implementation of GrDrawTarget that queues up draws for eventual
 * playback into a GrGpu. In theory one draw buffer could playback into another. When index or
 * vertex buffers are used as geometry sources it is the callers the draw buffer only holds
 * references to the buffers. It is the callers responsibility to ensure that the data is still
 * valid when the draw buffer is played back into a GrGpu. Similarly, it is the caller's
 * responsibility to ensure that all referenced textures, buffers, and render-targets are associated
 * in the GrGpu object that the buffer is played back into. The buffer requires VB and IB pools to
 * store geometry.
 */
class GrInOrderDrawBuffer : public GrFlushToGpuDrawTarget {
public:

    /**
     * Creates a GrInOrderDrawBuffer
     *
     * @param gpu        the gpu object that this draw buffer flushes to.
     * @param vertexPool pool where vertices for queued draws will be saved when
     *                   the vertex source is either reserved or array.
     * @param indexPool  pool where indices for queued draws will be saved when
     *                   the index source is either reserved or array.
     */
    GrInOrderDrawBuffer(GrGpu* gpu,
                        GrVertexBufferAllocPool* vertexPool,
                        GrIndexBufferAllocPool* indexPool);

    ~GrInOrderDrawBuffer() SK_OVERRIDE;

    // tracking for draws
    DrawToken getCurrentDrawToken() SK_OVERRIDE { return DrawToken(this, fDrawID); }

    void clearStencilClip(const SkIRect& rect,
                          bool insideClip,
                          GrRenderTarget* renderTarget) SK_OVERRIDE;

    void discard(GrRenderTarget*) SK_OVERRIDE;

protected:
    void willReserveVertexAndIndexSpace(int vertexCount,
                                        size_t vertexStride,
                                        int indexCount);

private:
    typedef GrGpu::DrawArgs DrawArgs;
    enum {
        kDraw_Cmd              = 1,
        kStencilPath_Cmd       = 2,
        kSetState_Cmd          = 3,
        kClear_Cmd             = 4,
        kCopySurface_Cmd       = 5,
        kDrawPath_Cmd          = 6,
        kDrawPaths_Cmd         = 7,
        kDrawBatch_Cmd         = 8,
    };

    struct SetState;

    struct Cmd : ::SkNoncopyable {
        Cmd(uint8_t type) : fType(type) {}
        virtual ~Cmd() {}

        virtual void execute(GrInOrderDrawBuffer*, const SetState*) = 0;

        uint8_t fType;
    };

    struct Draw : public Cmd {
        Draw(const DrawInfo& info) : Cmd(kDraw_Cmd), fInfo(info) {}

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        DrawInfo     fInfo;
    };

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path, GrRenderTarget* rt)
            : Cmd(kStencilPath_Cmd)
            , fRenderTarget(rt)
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        SkMatrix                                                fViewMatrix;
        bool                                                    fUseHWAA;
        GrStencilSettings                                       fStencil;
        GrScissorState                                          fScissor;
    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
        GrPendingIOResource<const GrPath, kRead_GrIOType>       fPath;
    };

    struct DrawPath : public Cmd {
        DrawPath(const GrPath* path) : Cmd(kDrawPath_Cmd), fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        GrStencilSettings       fStencilSettings;

    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(const GrPathRange* pathRange) : Cmd(kDrawPaths_Cmd), fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        int                     fIndicesLocation;
        PathIndexType           fIndexType;
        int                     fTransformsLocation;
        PathTransformType       fTransformType;
        int                     fCount;
        GrStencilSettings       fStencilSettings;

    private:
        GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public Cmd {
        Clear(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        SkIRect fRect;
        GrColor fColor;
        bool    fCanIgnoreRect;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    // This command is ONLY used by the clip mask manager to clear the stencil clip bits
    struct ClearStencilClip : public Cmd {
        ClearStencilClip(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src) : Cmd(kCopySurface_Cmd), fDst(dst), fSrc(src) {}

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
        GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    };

    // TODO: rename to SetPipeline once pp, batch tracker, and desc are removed
    struct SetState : public Cmd {
        // TODO get rid of the prim proc parameter when we use batch everywhere
        SetState(const GrPrimitiveProcessor* primProc = NULL)
        : Cmd(kSetState_Cmd)
        , fPrimitiveProcessor(primProc) {}

        ~SetState() { reinterpret_cast<GrPipeline*>(fPipeline.get())->~GrPipeline(); }

        // This function is only for getting the location in memory where we will create our
        // pipeline object.
        GrPipeline* pipelineLocation() { return reinterpret_cast<GrPipeline*>(fPipeline.get()); }

        const GrPipeline* getPipeline() const {
            return reinterpret_cast<const GrPipeline*>(fPipeline.get());
        }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor               fPrimitiveProcessor;
        SkAlignedSStorage<sizeof(GrPipeline)>   fPipeline;
        GrProgramDesc                           fDesc;
        GrBatchTracker                          fBatchTracker;
    };

    struct DrawBatch : public Cmd {
        DrawBatch(GrBatch* batch) : Cmd(kDrawBatch_Cmd), fBatch(SkRef(batch)) {
            SkASSERT(!batch->isUsed());
        }

        void execute(GrInOrderDrawBuffer*, const SetState*) SK_OVERRIDE;

        // TODO it wouldn't be too hard to let batches allocate in the cmd buffer
        SkAutoTUnref<GrBatch>  fBatch;
    };

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    void onReset() SK_OVERRIDE;
    void onFlush() SK_OVERRIDE;

    // overrides from GrDrawTarget
    void onDraw(const GrGeometryProcessor*, const DrawInfo&, const PipelineInfo&) SK_OVERRIDE;
    void onDrawBatch(GrBatch*, const PipelineInfo&) SK_OVERRIDE;
    void onDrawRect(GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) SK_OVERRIDE;

    void onStencilPath(const GrPipelineBuilder&,
                       const GrPathProcessor*,
                       const GrPath*,
                       const GrScissorState&,
                       const GrStencilSettings&) SK_OVERRIDE;
    void onDrawPath(const GrPathProcessor*,
                    const GrPath*,
                    const GrStencilSettings&,
                    const PipelineInfo&) SK_OVERRIDE;
    void onDrawPaths(const GrPathProcessor*,
                     const GrPathRange*,
                     const void* indices,
                     PathIndexType,
                     const float transformValues[],
                     PathTransformType,
                     int count,
                     const GrStencilSettings&,
                     const PipelineInfo&) SK_OVERRIDE;
    void onClear(const SkIRect* rect,
                 GrColor color,
                 bool canIgnoreRect,
                 GrRenderTarget* renderTarget) SK_OVERRIDE;
    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) SK_OVERRIDE;

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(const DrawInfo&);

    // Determines whether the current draw operation requires a new GrPipeline and if so
    // records it. If the draw can be skipped false is returned and no new GrPipeline is
    // recorded.
    // TODO delete the primproc variant when we have batches everywhere
    bool SK_WARN_UNUSED_RESULT setupPipelineAndShouldDraw(const GrPrimitiveProcessor*,
                                                          const PipelineInfo&);
    bool SK_WARN_UNUSED_RESULT setupPipelineAndShouldDraw(GrBatch*, const PipelineInfo&);

    // We lazily record clip changes in order to skip clips that have no effect.
    void recordClipIfNecessary();
    // Records any trace markers for a command after adding it to the buffer.
    void recordTraceMarkersIfNecessary();

    bool isIssued(uint32_t drawID) SK_OVERRIDE { return drawID != fDrawID; }

    GrBatchTarget* getBatchTarget() { return &fBatchTarget; }

    // TODO: Use a single allocator for commands and records
    enum {
        kCmdBufferInitialSizeInBytes = 8 * 1024,
        kPathIdxBufferMinReserve     = 2 * 64,  // 64 uint16_t's
        kPathXformBufferMinReserve   = 2 * 64,  // 64 two-float transforms
    };

    CmdBuffer                           fCmdBuffer;
    SetState*                           fPrevState;
    SkTArray<GrTraceMarkerSet, false>   fGpuCmdMarkers;
    SkTDArray<char>                     fPathIndexBuffer;
    SkTDArray<float>                    fPathTransformBuffer;
    uint32_t                            fDrawID;
    GrBatchTarget                       fBatchTarget;
    // TODO hack until batch is everywhere
    DrawBatch*                          fDrawBatch;

    // This will go away when everything uses batch.  However, in the short term anything which
    // might be put into the GrInOrderDrawBuffer needs to make sure it closes the last batch
    void closeBatch() {
        if (fDrawBatch) {
            fBatchTarget.resetNumberOfDraws();
            fDrawBatch->execute(this, fPrevState);
            fDrawBatch->fBatch->setNumberOfDraws(fBatchTarget.numberOfDraws());
            fDrawBatch = NULL;
        }
    }

    typedef GrFlushToGpuDrawTarget INHERITED;
};

#endif
