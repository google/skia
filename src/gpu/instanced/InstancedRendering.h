/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstancedRendering_DEFINED
#define gr_instanced_InstancedRendering_DEFINED

#include "../private/GrInstancedPipelineInfo.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "SkTInternalLList.h"
#include "instanced/InstancedRenderingTypes.h"
#include "ops/GrDrawOp.h"

class GrResourceProvider;

namespace gr_instanced {

class InstanceProcessor;

/**
 * This class serves as a centralized clearinghouse for instanced rendering. It accumulates data for
 * instanced draws into one location, and creates special ops that pull from this data. The
 * nature of instanced rendering allows these ops to combine well and render efficiently.
 *
 * During a flush, this class assembles the accumulated draw data into a single vertex and texel
 * buffer, and its subclass draws the ops using backend-specific instanced rendering APIs.
 *
 * This class is responsible for the CPU side of instanced rendering. Shaders are implemented by
 * InstanceProcessor.
 */
class InstancedRendering : public SkNoncopyable {
public:
    virtual ~InstancedRendering() { SkASSERT(State::kRecordingDraws == fState); }

    GrGpu* gpu() const { return fGpu.get(); }

    /**
     * These methods make a new record internally for an instanced draw, and return an op that is
     * effectively just an index to that record. The returned op is not self-contained, but
     * rather relies on this class to handle the rendering. The client must call beginFlush() on
     * this class before attempting to flush ops returned by it. It is invalid to record new
     * draws between beginFlush() and endFlush().
     */
    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, const SkRect& localRect,
                                                               GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&,
                                                               GrPaint&&,
                                                               const SkMatrix& localMatrix, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordOval(const SkRect&, const SkMatrix&,
                                                               GrPaint&&, GrAA,
                                                               const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordRRect(const SkRRect&, const SkMatrix&,
                                                                GrPaint&&, GrAA,
                                                                const GrInstancedPipelineInfo&);

    std::unique_ptr<GrDrawOp> SK_WARN_UNUSED_RESULT recordDRRect(const SkRRect& outer,
                                                                 const SkRRect& inner,
                                                                 const SkMatrix&, GrPaint&&, GrAA,
                                                                 const GrInstancedPipelineInfo&);

    /**
     * Compiles all recorded draws into GPU buffers and allows the client to begin flushing the
     * ops created by this class.
     */
    void beginFlush(GrResourceProvider*);

    /**
     * Called once the ops created previously by this class have all been released. Allows the
     * client to begin recording draws again.
     */
    void endFlush();

    enum class ResetType : bool {
        kDestroy,
        kAbandon
    };

    /**
     * Resets all GPU resources, including those that are held long term. They will be lazily
     * reinitialized if the class begins to be used again.
     */
    void resetGpuResources(ResetType);

protected:
    class Op : public GrDrawOp {
    public:
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Op);

        ~Op() override;
        const char* name() const override { return "InstancedRendering::Op"; }

        SkString dumpInfo() const override {
            SkString string;
            string.printf(
                    "AA: %d, ShapeTypes: 0x%02x, IShapeTypes: 0x%02x, Persp %d, "
                    "NonSquare: %d, PLoad: %0.2f, Tracked: %d, NumDraws: %d, "
                    "GeomChanges: %d\n",
                    (unsigned)fInfo.fAAType,
                    fInfo.fShapeTypes,
                    fInfo.fInnerShapeTypes,
                    fInfo.fHasPerspective,
                    fInfo.fNonSquare,
                    fPixelLoad,
                    fIsTracked,
                    fNumDraws,
                    fNumChangesInGeometry);
            string.append(INHERITED::dumpInfo());
            return string;
        }

        struct Draw {
            Instance     fInstance;
            IndexRange   fGeometry;
            Draw*        fNext;
        };

        Draw& getSingleDraw() const { SkASSERT(fHeadDraw && !fHeadDraw->fNext); return *fHeadDraw; }
        Instance& getSingleInstance() const { return this->getSingleDraw().fInstance; }

        void appendRRectParams(const SkRRect&);
        void appendParamsTexel(const SkScalar* vals, int count);
        void appendParamsTexel(SkScalar x, SkScalar y, SkScalar z, SkScalar w);
        void appendParamsTexel(SkScalar x, SkScalar y, SkScalar z);
        FixedFunctionFlags fixedFunctionFlags() const override {
            return GrAATypeIsHW(fInfo.aaType()) ? FixedFunctionFlags::kUsesHWAA
                                                : FixedFunctionFlags::kNone;
        }
        bool xpRequiresDstTexture(const GrCaps&, const GrAppliedClip*) override;

        // Registers the op with the InstancedRendering list of tracked ops.
        void wasRecorded() override;

    protected:
        Op(uint32_t classID, GrPaint&&, InstancedRendering*);

        InstancedRendering* const fInstancedRendering;
        OpInfo fInfo;
        SkScalar fPixelLoad;
        GrProcessorSet fProcessors;
        SkSTArray<5, ParamsTexel, true> fParams;
        bool fIsTracked : 1;
        bool fRequiresBarrierOnOverlap : 1;
        int fNumDraws;
        int fNumChangesInGeometry;
        Draw* fHeadDraw;
        Draw* fTailDraw;

    private:
        bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override;
        void onPrepare(GrOpFlushState*) override {}
        void onExecute(GrOpFlushState*) override;

        typedef GrDrawOp INHERITED;

        friend class InstancedRendering;
    };

    typedef SkTInternalLList<Op> OpList;

    InstancedRendering(GrGpu* gpu);

    const OpList& trackedOps() const { return fTrackedOps; }
    const GrBuffer* vertexBuffer() const { SkASSERT(fVertexBuffer); return fVertexBuffer.get(); }
    const GrBuffer* indexBuffer() const { SkASSERT(fIndexBuffer); return fIndexBuffer.get(); }

    virtual void onBeginFlush(GrResourceProvider*) = 0;
    virtual void onDraw(const GrPipeline&, const InstanceProcessor&, const Op*) = 0;
    virtual void onEndFlush() = 0;
    virtual void onResetGpuResources(ResetType) = 0;

private:
    enum class State : bool {
        kRecordingDraws,
        kFlushing
    };

    std::unique_ptr<Op> SK_WARN_UNUSED_RESULT recordShape(ShapeType, const SkRect& bounds,
                                                          const SkMatrix& viewMatrix, GrPaint&&,
                                                          const SkRect& localRect, GrAA aa,
                                                          const GrInstancedPipelineInfo&);

    bool selectAntialiasMode(const SkMatrix& viewMatrix, GrAA aa, const GrInstancedPipelineInfo&,
                             GrAAType*);

    virtual std::unique_ptr<Op> makeOp(GrPaint&&) = 0;

    const sk_sp<GrGpu> fGpu;
    State fState;
    GrObjectMemoryPool<Op::Draw> fDrawPool;
    SkSTArray<1024, ParamsTexel, true> fParams;
    OpList fTrackedOps;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<GrBuffer> fParamsBuffer;
};

}

#endif
