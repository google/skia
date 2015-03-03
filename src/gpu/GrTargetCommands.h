/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTargetCommands_DEFINED
#define GrTargetCommands_DEFINED

#include "GrBatchTarget.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPendingProgramElement.h"
#include "GrRenderTarget.h"
#include "GrTRecorder.h"
#include "SkRect.h"
#include "SkTypes.h"

class GrInOrderDrawBuffer;
class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;

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

#endif

