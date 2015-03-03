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
#include "SkChunkAlloc.h"
#include "GrPipeline.h"
#include "GrPath.h"
#include "GrTRecorder.h"

class GrInOrderDrawBuffer;

class GrTargetCommands : ::SkNoncopyable {
    struct SetState;

public:
    GrTargetCommands(GrGpu* gpu,
                     GrVertexBufferAllocPool* vertexPool,
                     GrIndexBufferAllocPool* indexPool)
        : fCmdBuffer(kCmdBufferInitialSizeInBytes)
        , fPrevState(NULL)
        , fBatchTarget(gpu, vertexPool, indexPool)
        , fDrawBatch(NULL) {
    }

    struct Cmd : ::SkNoncopyable {
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

        Cmd(uint8_t type) : fType(type) {}
        virtual ~Cmd() {}

        virtual void execute(GrGpu*, const SetState*) = 0;

        uint8_t type() const { return fType & kCmdMask; }

        bool isTraced() const { return SkToBool(fType & kTraceCmdBit); }
        void makeTraced() { fType |= kTraceCmdBit; }

    private:
        static const int kCmdMask = 0x7F;
        static const int kTraceCmdBit = 0x80;

        uint8_t fType;
    };

    void reset();
    void flush(GrInOrderDrawBuffer*);

    Cmd* recordClearStencilClip(GrInOrderDrawBuffer*,
                                const SkIRect& rect,
                                bool insideClip,
                                GrRenderTarget* renderTarget);

    Cmd* recordDiscard(GrInOrderDrawBuffer*, GrRenderTarget*);

    Cmd* recordDraw(GrInOrderDrawBuffer*,
                    const GrGeometryProcessor*,
                    const GrDrawTarget::DrawInfo&,
                    const GrDrawTarget::PipelineInfo&);
    Cmd* recordDrawBatch(GrInOrderDrawBuffer*,
                         GrBatch*,
                         const GrDrawTarget::PipelineInfo&);
    void recordDrawRect(GrInOrderDrawBuffer*,
                        GrPipelineBuilder*,
                        GrColor,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const SkRect* localRect,
                        const SkMatrix* localMatrix);
    Cmd* recordStencilPath(GrInOrderDrawBuffer*,
                           const GrPipelineBuilder&,
                           const GrPathProcessor*,
                           const GrPath*,
                           const GrScissorState&,
                           const GrStencilSettings&);
    Cmd* recordDrawPath(GrInOrderDrawBuffer*,
                        const GrPathProcessor*,
                        const GrPath*,
                        const GrStencilSettings&,
                        const GrDrawTarget::PipelineInfo&);
    Cmd* recordDrawPaths(GrInOrderDrawBuffer*,
                         const GrPathProcessor*,
                         const GrPathRange*,
                         const void*,
                         GrDrawTarget::PathIndexType,
                         const float transformValues[],
                         GrDrawTarget::PathTransformType ,
                         int,
                         const GrStencilSettings&,
                         const GrDrawTarget::PipelineInfo&);
    Cmd* recordClear(GrInOrderDrawBuffer*,
                     const SkIRect* rect,
                     GrColor,
                     bool canIgnoreRect,
                     GrRenderTarget*);
    Cmd* recordCopySurface(GrInOrderDrawBuffer*,
                           GrSurface* dst,
                           GrSurface* src,
                           const SkIRect& srcRect,
                           const SkIPoint& dstPoint);

protected:
    void willReserveVertexAndIndexSpace(int vertexCount,
                                        size_t vertexStride,
                                        int indexCount);

private:
    friend class GrInOrderDrawBuffer;

    typedef GrGpu::DrawArgs DrawArgs;

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(GrInOrderDrawBuffer*, const GrDrawTarget::DrawInfo&);

    bool SK_WARN_UNUSED_RESULT setupPipelineAndShouldDraw(GrInOrderDrawBuffer*,
                                                          const GrPrimitiveProcessor*,
                                                          const GrDrawTarget::PipelineInfo&);
    bool SK_WARN_UNUSED_RESULT setupPipelineAndShouldDraw(GrInOrderDrawBuffer*,
                                                          GrBatch*,
                                                          const GrDrawTarget::PipelineInfo&);

    struct Draw : public Cmd {
        Draw(const GrDrawTarget::DrawInfo& info) : Cmd(kDraw_Cmd), fInfo(info) {}

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        GrDrawTarget::DrawInfo     fInfo;
    };

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path, GrRenderTarget* rt)
            : Cmd(kStencilPath_Cmd)
            , fRenderTarget(rt)
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

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

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        GrStencilSettings       fStencilSettings;

    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(const GrPathRange* pathRange) : Cmd(kDrawPaths_Cmd), fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        char*                           fIndices;
        GrDrawTarget::PathIndexType     fIndexType;
        float*                          fTransforms;
        GrDrawTarget::PathTransformType fTransformType;
        int                             fCount;
        GrStencilSettings               fStencilSettings;

    private:
        GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public Cmd {
        Clear(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

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

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src) : Cmd(kCopySurface_Cmd), fDst(dst), fSrc(src) {}

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

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

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor               fPrimitiveProcessor;
        SkAlignedSStorage<sizeof(GrPipeline)>   fPipeline;
        GrProgramDesc                           fDesc;
        GrBatchTracker                          fBatchTracker;
    };

    struct DrawBatch : public Cmd {
        DrawBatch(GrBatch* batch, GrBatchTarget* batchTarget) 
            : Cmd(kDrawBatch_Cmd)
            , fBatch(SkRef(batch))
            , fBatchTarget(batchTarget) {
            SkASSERT(!batch->isUsed());
        }

        void execute(GrGpu*, const SetState*) SK_OVERRIDE;

        // TODO it wouldn't be too hard to let batches allocate in the cmd buffer
        SkAutoTUnref<GrBatch>  fBatch;

    private:
        GrBatchTarget*         fBatchTarget;
    };

     static const int kCmdBufferInitialSizeInBytes = 8 * 1024;

     typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
     typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

     CmdBuffer                           fCmdBuffer;
     SetState*                           fPrevState;
     GrBatchTarget                       fBatchTarget;
     // TODO hack until batch is everywhere
     GrTargetCommands::DrawBatch*        fDrawBatch;

     // This will go away when everything uses batch.  However, in the short term anything which
     // might be put into the GrInOrderDrawBuffer needs to make sure it closes the last batch
     void closeBatch();
};

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
                                        int indexCount) SK_OVERRIDE;

    void appendIndicesAndTransforms(const void* indexValues, PathIndexType indexType, 
                                    const float* transformValues, PathTransformType transformType,
                                    int count, char** indicesLocation, float** xformsLocation) {
        int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
        *indicesLocation = (char*) fPathIndexBuffer.alloc(count * indexBytes,
                                                          SkChunkAlloc::kThrow_AllocFailType);
        SkASSERT(SkIsAlign4((uintptr_t)*indicesLocation));
        memcpy(*indicesLocation, reinterpret_cast<const char*>(indexValues), count * indexBytes);

        const int xformBytes = GrPathRendering::PathTransformSize(transformType) * sizeof(float);
        *xformsLocation = NULL;

        if (0 != xformBytes) {
            *xformsLocation = (float*) fPathTransformBuffer.alloc(count * xformBytes,
                                                               SkChunkAlloc::kThrow_AllocFailType);
            SkASSERT(SkIsAlign4((uintptr_t)*xformsLocation));
            memcpy(*xformsLocation, transformValues, count * xformBytes);
        }
    }

    bool canConcatToIndexBuffer(const GrIndexBuffer** ib) {
        const GrDrawTarget::GeometrySrcState& geomSrc = this->getGeomSrc();

        // we only attempt to concat when reserved verts are used with a client-specified
        // index buffer. To make this work with client-specified VBs we'd need to know if the VB
        // was updated between draws.
        if (kReserved_GeometrySrcType != geomSrc.fVertexSrc ||
            kBuffer_GeometrySrcType != geomSrc.fIndexSrc) {
            return false;
        }

        *ib = geomSrc.fIndexBuffer;
        return true;
    }

private:
    friend class GrTargetCommands;

    typedef GrGpu::DrawArgs DrawArgs;

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

    // We lazily record clip changes in order to skip clips that have no effect.
    void recordClipIfNecessary();
    // Records any trace markers for a command
    void recordTraceMarkersIfNecessary(GrTargetCommands::Cmd*);
    SkString getCmdString(int index) const {
        SkASSERT(index < fGpuCmdMarkers.count());
        return fGpuCmdMarkers[index].toString();
    }
    bool isIssued(uint32_t drawID) SK_OVERRIDE { return drawID != fDrawID; }

    // TODO: Use a single allocator for commands and records
    enum {
        kPathIdxBufferMinReserve     = 2 * 64,  // 64 uint16_t's
        kPathXformBufferMinReserve   = 2 * 64,  // 64 two-float transforms
    };

    GrTargetCommands                    fCommands;
    SkTArray<GrTraceMarkerSet, false>   fGpuCmdMarkers;
    SkChunkAlloc                        fPathIndexBuffer;
    SkChunkAlloc                        fPathTransformBuffer;
    uint32_t                            fDrawID;

    typedef GrFlushToGpuDrawTarget INHERITED;
};

#endif
