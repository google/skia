/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstancedRendering_DEFINED
#define gr_instanced_InstancedRendering_DEFINED

#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "SkTInternalLList.h"
#include "batches/GrDrawBatch.h"
#include "instanced/InstancedRenderingTypes.h"
#include "../private/GrInstancedPipelineInfo.h"

class GrResourceProvider;

namespace gr_instanced {

class InstanceProcessor;

/**
 * This class serves as a centralized clearinghouse for instanced rendering. It accumulates data for
 * instanced draws into one location, and creates special batches that pull from this data. The
 * nature of instanced rendering allows these batches to combine well and render efficiently.
 *
 * During a flush, this class assembles the accumulated draw data into a single vertex and texel
 * buffer, and its subclass draws the batches using backend-specific instanced rendering APIs.
 *
 * This class is responsible for the CPU side of instanced rendering. Shaders are implemented by
 * InstanceProcessor.
 */
class InstancedRendering : public SkNoncopyable {
public:
    virtual ~InstancedRendering() { SkASSERT(State::kRecordingDraws == fState); }

    GrGpu* gpu() const { return fGpu.get(); }

    /**
     * These methods make a new record internally for an instanced draw, and return a batch that is
     * effectively just an index to that record. The returned batch is not self-contained, but
     * rather relies on this class to handle the rendering. The client must call beginFlush() on
     * this class before attempting to flush batches returned by it. It is invalid to record new
     * draws between beginFlush() and endFlush().
     */
    GrDrawBatch* SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&, GrColor,
                                                  bool antialias, const GrInstancedPipelineInfo&,
                                                  bool* useHWAA);

    GrDrawBatch* SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&, GrColor,
                                                  const SkRect& localRect, bool antialias,
                                                  const GrInstancedPipelineInfo&, bool* useHWAA);

    GrDrawBatch* SK_WARN_UNUSED_RESULT recordRect(const SkRect&, const SkMatrix&, GrColor,
                                                  const SkMatrix& localMatrix, bool antialias,
                                                  const GrInstancedPipelineInfo&, bool* useHWAA);

    GrDrawBatch* SK_WARN_UNUSED_RESULT recordOval(const SkRect&, const SkMatrix&, GrColor,
                                                  bool antialias, const GrInstancedPipelineInfo&,
                                                  bool* useHWAA);

    GrDrawBatch* SK_WARN_UNUSED_RESULT recordRRect(const SkRRect&, const SkMatrix&, GrColor,
                                                   bool antialias, const GrInstancedPipelineInfo&,
                                                   bool* useHWAA);

    GrDrawBatch* SK_WARN_UNUSED_RESULT recordDRRect(const SkRRect& outer, const SkRRect& inner,
                                                    const SkMatrix&, GrColor, bool antialias,
                                                    const GrInstancedPipelineInfo&, bool* useHWAA);

    /**
     * Compiles all recorded draws into GPU buffers and allows the client to begin flushing the
     * batches created by this class.
     */
    void beginFlush(GrResourceProvider*);

    /**
     * Called once the batches created previously by this class have all been released. Allows the
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
    class Batch : public GrDrawBatch {
    public:
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Batch);

        ~Batch() override;
        const char* name() const override { return "Instanced Batch"; }

        SkString dumpInfo() const override {
            SkString string;
            string.printf("AA: %d, ShapeTypes: 0x%02x, IShapeTypes: 0x%02x, Persp %d, "
                          "NonSquare: %d, PLoad: %0.2f, Tracked: %d, NumDraws: %d, "
                          "GeomChanges: %d\n",
                          (int)fInfo.fAntialiasMode,
                          fInfo.fShapeTypes,
                          fInfo.fInnerShapeTypes,
                          fInfo.fHasPerspective,
                          fInfo.fNonSquare,
                          fPixelLoad,
                          fIsTracked,
                          fNumDraws,
                          fNumChangesInGeometry);
            string.append(DumpPipelineInfo(*this->pipeline()));
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

    protected:
        Batch(uint32_t classID, InstancedRendering* ir);

        void initBatchTracker(const GrXPOverridesForBatch&) override;
        bool onCombineIfPossible(GrBatch* other, const GrCaps& caps) override;

        void computePipelineOptimizations(GrInitInvariantOutput* color,
                                          GrInitInvariantOutput* coverage,
                                          GrBatchToXPOverrides*) const override;

        void onPrepare(GrBatchFlushState*) override {}
        void onDraw(GrBatchFlushState*, const SkRect& bounds) override;

        InstancedRendering* const         fInstancedRendering;
        BatchInfo                         fInfo;
        SkScalar                          fPixelLoad;
        SkSTArray<5, ParamsTexel, true>   fParams;
        bool                              fIsTracked;
        int                               fNumDraws;
        int                               fNumChangesInGeometry;
        Draw*                             fHeadDraw;
        Draw*                             fTailDraw;

        typedef GrDrawBatch INHERITED;

        friend class InstancedRendering;
    };

    typedef SkTInternalLList<Batch> BatchList;

    InstancedRendering(GrGpu* gpu);

    const BatchList& trackedBatches() const { return fTrackedBatches; }
    const GrBuffer* vertexBuffer() const { SkASSERT(fVertexBuffer); return fVertexBuffer.get(); }
    const GrBuffer* indexBuffer() const { SkASSERT(fIndexBuffer); return fIndexBuffer.get(); }

    virtual void onBeginFlush(GrResourceProvider*) = 0;
    virtual void onDraw(const GrPipeline&, const InstanceProcessor&, const Batch*) = 0;
    virtual void onEndFlush() = 0;
    virtual void onResetGpuResources(ResetType) = 0;

private:
    enum class State : bool {
        kRecordingDraws,
        kFlushing
    };

    Batch* SK_WARN_UNUSED_RESULT recordShape(ShapeType, const SkRect& bounds,
                                             const SkMatrix& viewMatrix, GrColor,
                                             const SkRect& localRect, bool antialias,
                                             const GrInstancedPipelineInfo&, bool* requireHWAA);

    bool selectAntialiasMode(const SkMatrix& viewMatrix, bool antialias,
                             const GrInstancedPipelineInfo&, bool* useHWAA, AntialiasMode*);

    virtual Batch* createBatch() = 0;

    const sk_sp<GrGpu>                   fGpu;
    State                                fState;
    GrObjectMemoryPool<Batch::Draw>      fDrawPool;
    SkSTArray<1024, ParamsTexel, true>   fParams;
    BatchList                            fTrackedBatches;
    sk_sp<const GrBuffer>                fVertexBuffer;
    sk_sp<const GrBuffer>                fIndexBuffer;
    sk_sp<GrBuffer>                      fParamsBuffer;
};

}

#endif
