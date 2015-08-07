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

#include "batches/GrBatch.h"

class GrBufferedDrawTarget;

class GrTargetCommands : ::SkNoncopyable {
public:
    GrTargetCommands(GrGpu* gpu)
        : fCmdBuffer(kCmdBufferInitialSizeInBytes)
        , fBatchTarget(gpu) {
    }

    class Cmd : ::SkNoncopyable {
    public:
        enum CmdType {
            kStencilPath_CmdType       = 1,
            kSetState_CmdType          = 2,
            kClear_CmdType             = 3,
            kClearStencil_CmdType      = 4,
            kCopySurface_CmdType       = 5,
            kDrawPath_CmdType          = 6,
            kDrawPaths_CmdType         = 7,
            kDrawBatch_CmdType         = 8,
            kXferBarrier_CmdType       = 9,
        };

        Cmd(CmdType type)
            : fMarkerID(-1)
            , fType(type)
#if GR_BATCH_SPEW
            , fUniqueID(GenID(&gUniqueID))
#endif
{}
        virtual ~Cmd() {}

        virtual void execute(GrGpu*) = 0;

        CmdType type() const { return fType; }

        // trace markers
        bool isTraced() const { return -1 != fMarkerID; }
        void setMarkerID(int markerID) { SkASSERT(-1 == fMarkerID); fMarkerID = markerID; }
        int markerID() const { return fMarkerID; }
        GrBATCH_SPEW(uint32_t uniqueID() const { return fUniqueID;} )

    private:
        // TODO move this to a common header so it can be shared with GrBatch
        static uint32_t GenID(int32_t* idCounter) {
            uint32_t id = static_cast<uint32_t>(sk_atomic_inc(idCounter)) + 1;
            if (!id) {
                SkFAIL("This should never wrap\n");
            }
            return id;
        }
        int              fMarkerID;
        CmdType          fType;
        GrBATCH_SPEW(uint32_t fUniqueID);
        GrBATCH_SPEW(static int32_t gUniqueID;)
    };

    void reset();
    void flush(GrBufferedDrawTarget*);

private:
    friend class GrCommandBuilder;
    friend class GrBufferedDrawTarget; // This goes away when State becomes just a pipeline
    friend class GrReorderCommandBuilder;

    typedef GrGpu::DrawArgs DrawArgs;

    void recordXferBarrierIfNecessary(const GrPipeline&, GrBufferedDrawTarget*);

    // TODO: This can be just a pipeline once paths are in batch, and it should live elsewhere
    struct State : public SkNVRefCnt<State> {
        // TODO get rid of the prim proc parameter when we use batch everywhere
        State(const GrPrimitiveProcessor* primProc = NULL)
            : fPrimitiveProcessor(primProc)
            , fCompiled(false) {}

        ~State() { reinterpret_cast<GrPipeline*>(fPipeline.get())->~GrPipeline(); }

        // This function is only for getting the location in memory where we will create our
        // pipeline object.
        void* pipelineLocation() { return fPipeline.get(); }

        const GrPipeline* getPipeline() const {
            return reinterpret_cast<const GrPipeline*>(fPipeline.get());
        }
        GrRenderTarget* getRenderTarget() const {
            return this->getPipeline()->getRenderTarget();
        }
        const GrXferProcessor* getXferProcessor() const {
            return this->getPipeline()->getXferProcessor();
        }

        void operator delete(void* p) {}
        void* operator new(size_t) {
            SkFAIL("All States are created by placement new.");
            return sk_malloc_throw(0);
        }

        void* operator new(size_t, void* p) { return p; }
        void operator delete(void* target, void* placement) {
            ::operator delete(target, placement);
        }

        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor               fPrimitiveProcessor;
        SkAlignedSStorage<sizeof(GrPipeline)>   fPipeline;
        GrProgramDesc                           fDesc;
        GrBatchTracker                          fBatchTracker;
        bool                                    fCompiled;
    };
    // TODO remove this when State is just a pipeline
    friend SkNVRefCnt<State>;

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path, GrRenderTarget* rt)
            : Cmd(kStencilPath_CmdType)
            , fRenderTarget(rt)
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrGpu*) override;

        SkMatrix                                                fViewMatrix;
        bool                                                    fUseHWAA;
        GrStencilSettings                                       fStencil;
        GrScissorState                                          fScissor;
    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
        GrPendingIOResource<const GrPath, kRead_GrIOType>       fPath;
    };

    struct DrawPath : public Cmd {
        DrawPath(State* state, const GrPath* path)
            : Cmd(kDrawPath_CmdType)
            , fState(SkRef(state))
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrGpu*) override;

        SkAutoTUnref<State>     fState;
        GrStencilSettings       fStencilSettings;
    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(State* state, const GrPathRange* pathRange)
            : Cmd(kDrawPaths_CmdType)
            , fState(SkRef(state))
            , fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        void execute(GrGpu*) override;

        SkAutoTUnref<State>             fState;
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
        Clear(GrRenderTarget* rt) : Cmd(kClear_CmdType), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrGpu*) override;

        SkIRect fRect;
        GrColor fColor;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    // This command is ONLY used by the clip mask manager to clear the stencil clip bits
    struct ClearStencilClip : public Cmd {
        ClearStencilClip(GrRenderTarget* rt) : Cmd(kClearStencil_CmdType), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrGpu*) override;

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src)
            : Cmd(kCopySurface_CmdType)
            , fDst(dst)
            , fSrc(src) {
        }

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        void execute(GrGpu*) override;

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
        GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    };

    struct DrawBatch : public Cmd {
        DrawBatch(State* state, GrBatch* batch, GrBatchTarget* batchTarget)
            : Cmd(kDrawBatch_CmdType)
            , fState(SkRef(state))
            , fBatch(SkRef(batch))
            , fBatchTarget(batchTarget) {
            SkASSERT(!batch->isUsed());
        }

        void execute(GrGpu*) override;

        SkAutoTUnref<State>    fState;
        SkAutoTUnref<GrBatch>  fBatch;

    private:
        GrBatchTarget*         fBatchTarget;
    };

    struct XferBarrier : public Cmd {
        XferBarrier(GrRenderTarget* rt)
            : Cmd(kXferBarrier_CmdType)
            , fRenderTarget(rt) {
        }

        void execute(GrGpu*) override;

        GrXferBarrierType   fBarrierType;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    static const int kCmdBufferInitialSizeInBytes = 8 * 1024;

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    CmdBuffer* cmdBuffer() { return &fCmdBuffer; }
    GrBatchTarget* batchTarget() { return &fBatchTarget; }

    CmdBuffer                           fCmdBuffer;
    GrBatchTarget                       fBatchTarget;
};

#endif

